#pragma once

#include "DesignImpl.h"
#include "../Includes/Design.h"

namespace AudIO
{

    DesignImpl::DesignImpl( uint32_t nIn, uint32_t nOut )
        : _inputCount( nIn ), _outputCount( nOut )
    {}

    void DesignImpl::prepareToPlay( uint32_t sampleRate, uint32_t blockSize )
    {
        _sampleRate = sampleRate;
        _blockSize  = blockSize;

        topologicalSort();

        for( auto obj : _dfs )
        {
            obj->prepareToPlay( _sampleRate, _blockSize );
        }

        _meters.prepareToPlay( _sampleRate, _blockSize );
        _running.store( true );
    }

    inline void DesignImpl::process( Buffer<float>& Buffer )
    {
        auto dInGain     = designInputGain.getNext( _blockSize );
        auto dOutGain    = designOutputGain.getNext( _blockSize );
        auto dBypassGain = bypassGain.getNext( _blockSize );

        handleClearDesign(); // delete everything if marked for clear
        processRemoveQueue(); // delete any objects marked for remove
        topologicalSort(); // sort design for processing
        handleClearDesignInput();
        handleClearDesignOutput();

        if( utils::isEqual<float>( 0.f, dOutGain ) )
        {
            Buffer.clearOutputs();
        }
        else
        {
            if( utils::isEqual<float>( 0.f, dBypassGain ) )
            {
                Buffer.copyInputsToOutputs();
            }
            else
            {
                // Update Inputs
                if( auto inBuffer = _inputObjBuffer.load() )
                {
                    for( uint32_t ch = 0; ch < _inputCount; ch++ )
                        inBuffer->patchInput( Buffer.getInputBuffer( ch ), ch );
                }

                // Process design
                for( auto obj : _dfs )
                {
                    if( obj )
                        obj->process();
                }

                // Copy Outputs
                if( auto outBuffer = _outputObjBuffer.load() )
                {
                    auto size = outBuffer->getBlockSize();
                    for( uint32_t ch = 0; ch < _outputCount; ch++ )
                    {
                        if( auto dst = Buffer.getOutputBuffer( ch ) )
                        {
                            auto src = outBuffer->getOutputBuffer( ch );
                            utils::copyWithGain( dst, src, size, dOutGain );
                        }
                    }
                }
            } // Not Bypass
        }
        _meters.process( _metersbuff );
    }

    bool DesignImpl::setDesignInput( uint32_t objid, bool Force )
    {
        if( auto dstobj = getObject( objid ); !dstobj )
            return false;

        auto& meta   = _objects.at( objid );
        auto& buffer = meta.getBuffer();
        _inputObjBuffer.store( &buffer );

        _clearDesignInput.store( false );
        designInputGain.store( 1.f, Force );
        return true;
    }

    bool DesignImpl::setDesignOutput( uint32_t objid, bool Force )
    {
        if( auto dstobj = getObject( objid ); !dstobj )
            return false;

        auto& meta   = _objects.at( objid );
        auto& buffer = meta.getBuffer();
        _outputObjBuffer.store( &buffer );

        _clearDesignOutput.store( false );
        designOutputGain.store( 1.f, Force );
        return true;
    }

    void DesignImpl::clearDesignInput( bool Force )
    {
        designInputGain.store( 0.f, ( Force || !_running ) ? true : false );
        _clearDesignInput.store( true );
        if( !_running )
        {
            handleClearDesignInput();
        }
    }

    void DesignImpl::clearDesignOutput( bool Force )
    {
        if( _running )
        {
            designOutputGain.store( 0.f, Force );
            _clearDesignOutput.store( true );
        }
        else
            _outputObjBuffer.store( nullptr );
    }

    void DesignImpl::setDefaults()
    {
        designInputGain.store( 0.0f, true );
        designOutputGain.store( 0.0f, true );
        bypassGain.store( 1.0f, true );

        _clearDesignInput  = false;
        _clearDesignOutput = false;
        _clearRequest      = false;
    }

    inline void DesignImpl::clear()
    {
        _clearRequest.store( true );
        if( !_running.load() )
            handleClearDesign();
    }

    bool DesignImpl::patch( objectMeta& srcmeta, const uint32_t srcch, objectMeta& dstmeta, const uint32_t dstch )
    {
        if( srcmeta.patch( dstmeta, srcch, dstch ) )
        {
            if( isCyclic() )
            {
                srcmeta.unpatch( dstmeta, srcch, dstch );
                return false;
            }
            _dirty.store( true );
            return true;
        }
        return false;
    }

    bool DesignImpl::patch( uint32_t srcid, uint32_t srcch, uint32_t dstid, uint32_t dstch )
    {
        auto srcMeta = getObjectMeta( srcid );
        auto dstMeta = getObjectMeta( dstid );
        if( srcMeta && dstMeta )
        {
            return patch( *srcMeta, srcch, *dstMeta, dstch );
        }
        return false;
    }

    bool DesignImpl::unpatch( uint32_t srcid, uint32_t srcch, uint32_t dstid, uint32_t dstch )
    {
        auto srcMeta = getObjectMeta( srcid );
        auto dstMeta = getObjectMeta( dstid );
        if( srcMeta && dstMeta )
        {
            return srcMeta->unpatch( *dstMeta, srcch, dstch );
        }
        return false;
    }

    bool DesignImpl::insertObject( std::unique_ptr<Object> obj, uint32_t sampleRate, uint32_t blockSize )
    {
        if( obj )
        {
            auto& rObj = *obj.get();
            _objectPointers.push_back( std::move( obj ) );
            return insertObject( rObj, sampleRate, blockSize );
        }
        return false;
    }

    bool DesignImpl::insertObject( Object& obj, uint32_t sampleRate, uint32_t blockSize )
    {
        if( _objects.find( obj.getId() ) == _objects.end() )
        {
            _objects.emplace( obj.getId(), obj );
            if( _running )
            {
                obj.prepareToPlay( sampleRate, blockSize );
            }
            _dirty.store( true );
            return true;
        }
        return false;
    }

    bool DesignImpl::removeObject( objectMeta* objMeta )
    {
        if( objMeta )
        {
            auto id = objMeta->_object.getId();
            for( auto& [key, value] : _objects )
            {
                if( value._object.getId() != id )
                {
                    value.removeConnectionsTo( objMeta );
                }
            }

            _dirty.store( true );
            if( _running )
            {
                _removeQueue.push( id );
                return true;
            }
            else
            {
                return handleRemove( id );
            }
        }
        return false;
    }

    uint32_t DesignImpl::insertDesign( std::shared_ptr<Design> obj )
    {
        if( obj )
        {
            if( insertObject( *obj, _sampleRate, _blockSize ) )
            {
                _designs.emplace_back( obj );
                _dirty.store( true );
                return obj->getId();
            }
        }
        return 0;
    }

    std::shared_ptr<Design> DesignImpl::getDesign( uint32_t id )
    {
        auto find = [id]( const std::shared_ptr<Design> design ) {
            return design->getId() == id;
        };

        auto it = std::find_if( _designs.begin(), _designs.end(), find );
        if( it != _designs.end() )
        {
            return *it;
        }
        return nullptr;
    }

    Chain* DesignImpl::createChain( bool Stereo )
    {
        Chain* chain{ nullptr };

        auto pChain = std::make_unique<Chain>( Stereo );
        if( pChain )
        {
            chain = pChain.get();

            _chains.push_back( std::move( pChain ) );
            if( insertObject( *chain, _sampleRate, _blockSize ) )
            {
                if( auto m = _getObjectMeta( _objects, chain->getId() ) )
                {
                    chain->initializeChain( m );
                }
            }
        }
        return chain;
    }

    Bus* DesignImpl::createBus()
    {
        Bus* bus{ nullptr };

        if( auto pBus = std::make_unique<Bus>() )
        {
            bus = pBus.get();
            _buses.push_back( std::move( pBus ) );

            if( insertObject( *bus, _sampleRate, _blockSize ) )
            {
                if( auto m = _getObjectMeta( _objects, bus->getId() ) )
                {
                    bus->initializeBus( m );
                }
            }
        }

        return bus;
    }

    bool DesignImpl::isCyclic()
    {
        for( auto& [id, objMeta] : _objects )
        {
            objMeta.clearVisited();
            objMeta.clearDfs();
        }
        for( auto& [id, objMeta] : _objects )
        {
            if( checkCycles( objMeta ) )
                return true;
        }
        return false;
    }

    void DesignImpl::topologicalSort()
    {
        if( !_dirty.load() )
            return;
        for( auto& [id, objMeta] : _objects )
        {
            objMeta.clearVisited();
            objMeta.clearDfs();
        }
        _dfs.resize( _objects.size() );
        _dfsIndex = (uint32_t)_dfs.size() - 1;
        for( auto& [id, objMeta] : _objects )
        {
            topologicalSortUtil( objMeta );
        }
#if defined( DEBUG_PRINTS ) && ( DEBUG_PRINTS > 0 )
        // for( auto obj : _dfs )
        // {
        //     if( obj )
        //         std::cout << obj->getObject().getName() << ", ";
        // }
        // std::cout << std::endl;
#endif
        _dirty.store( false );
    }

    void DesignImpl::topologicalSortUtil( objectMeta& meta )
    {
        if( !meta.visited() )
        {
            meta.markVisited();
            bool flag = false;
            auto func = [&]( objectMeta& adj ) {
                topologicalSortUtil( adj );
            };
            meta.visitAll( func, flag );
            _dfs[_dfsIndex] = ( &meta );
            if( _dfsIndex )
                _dfsIndex--;
        }
    }

    bool DesignImpl::checkCycles( objectMeta& meta )
    {
        bool flag = false;
        if( !meta.visited() )
        {
            meta.markVisited();
            meta.markDfs();
            auto func = [&]( objectMeta& adj ) {
                if( checkCycles( adj ) )
                {
                    flag = true;
                }
                else if( adj.dfs() )
                {
                    flag = true;
                }
            };
            meta.visitAll( func, flag );
        }
        if( !flag )
            meta.clearDfs();

        return flag;
    }

    bool DesignImpl::handleRemove( uint32_t id )
    {
        auto it = _objects.find( id );
        if( it != _objects.end() )
        {
            // If a meter is added on this object remove it now, otherwise it will be left dangling.
            _meters.removeMeter( id );

            auto findindesign = [id]( const std::shared_ptr<Design> design ) {
                return design->getId() == id;
            };
            auto designit = std::find_if( _designs.begin(), _designs.end(), findindesign );
            if( designit != _designs.end() )
            {
                _designs.erase( designit );
            }
            else
            {
                auto findinChain = [id]( const std::unique_ptr<Chain>& chain ) {
                    return chain->getId() == id;
                };
                auto chainit = std::find_if( _chains.begin(), _chains.end(), findinChain );
                if( chainit != _chains.end() )
                {
                    _chains.erase( chainit );
                }
                else
                {
                    auto findinBus = [id]( const std::unique_ptr<Bus>& bus ) {
                        return bus->getId() == id;
                    };
                    auto busit = std::find_if( _buses.begin(), _buses.end(), findinBus );
                    if( busit != _buses.end() )
                    {
                        _buses.erase( busit );
                    }
                    else
                    {
                        auto findinObjects = [id]( const std::unique_ptr<Object>& obj ) {
                            return obj->getId() == id;
                        };
                        auto objit = std::find_if( _objectPointers.begin(), _objectPointers.end(), findinObjects );
                        if( objit != _objectPointers.end() )
                        {
                            _objectPointers.erase( objit );
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
            }

            _objects.erase( id );
            _dirty.store( true );

#if defined( DEBUG_PRINTS ) && ( DEBUG_PRINTS > 0 )
            std::cout << "Removed object " << std::endl;
#endif
            return true;
        }
        return false;
    }

    void DesignImpl::processRemoveQueue()
    {
        uint32_t id;
        while( true == _removeQueue.pop( id ) )
        {
            handleRemove( id );
        }
    }

    inline void DesignImpl::handleClearDesign()
    {
        if( _clearRequest.load() )
        {
            _dirty           = true;
            _inputObjBuffer  = nullptr;
            _outputObjBuffer = nullptr;

            _meters.clear();
            _designs.clear();
            _chains.clear();
            _buses.clear();
            _objects.clear();
            _objectPointers.clear();

            processRemoveQueue();
            topologicalSort();

            setDefaults();
        }
    }

    inline void DesignImpl::handleClearDesignInput()
    {
        if( _clearDesignInput.load() )
        {
            auto dInGain = designInputGain.getCurrent();
            if( utils::isEqual<float>( 0.f, dInGain ) )
            {
                if( auto inBuffer = _inputObjBuffer.load() )
                {
                    for( uint32_t ch = 0; ch < _inputCount; ch++ )
                        inBuffer->patchInput( nullptr, ch );
                }
                _inputObjBuffer.store( nullptr );
                _clearDesignInput.store( false );
            }
        }
    }

    inline void DesignImpl::handleClearDesignOutput()
    {
        if( _clearDesignOutput.load() )
        {
            auto dOutGain = designOutputGain.getCurrent();
            if( utils::isEqual<float>( 0.f, dOutGain ) )
            {
                _outputObjBuffer.store( nullptr );
                _clearDesignOutput.store( false );
            }
        }
    }

} // Namespace AudIO
