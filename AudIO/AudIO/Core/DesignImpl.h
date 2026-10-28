#pragma once

#include "../Includes/Platform.h"

#include "Bus.h"
#include "MeterEngine.h"

#include <atomic>
#include <list>

namespace AudIO
{

    // STATIC helper functions...
    inline bool _find( objectMeta& meta, std::list<objectMeta*>& l )
    {
        return std::find( l.begin(), l.end(), &meta ) != l.end();
    }

    static Object* _getObject( std::unordered_map<uint32_t, objectMeta>& objects, uint32_t id )
    {
        auto it = objects.find( id );
        return ( it != objects.end() ) ? &it->second.getObject() : nullptr;
    }

    static objectMeta* _getObjectMeta( std::unordered_map<uint32_t, objectMeta>& objects, uint32_t id )
    {
        auto it = objects.find( id );
        return ( it != objects.end() ) ? &it->second : nullptr;
    }

    static Buffer<float>* _getObjectBuffer( std::unordered_map<uint32_t, objectMeta>& objects, const uint32_t objId )
    {
        auto it = objects.find( objId );
        return ( it != objects.end() ) ? &it->second.getBuffer() : nullptr;
    }

    static Chain* _getChain( std::list<std::unique_ptr<Chain>>& chains, const uint32_t chainorbusid )
    {
        auto chainIt = std::find_if( chains.begin(), chains.end(), [chainorbusid]( const std::unique_ptr<Chain>& chain ) {
            return chain->getId() == chainorbusid;
        } );
        return ( chainIt != chains.end() ) ? chainIt->get() : nullptr;
    }

    static Bus* _getBus( std::list<std::unique_ptr<Bus>>& buses, const uint32_t chainorbusid )
    {
        auto busIt = std::find_if( buses.begin(), buses.end(), [chainorbusid]( const std::unique_ptr<Bus>& bus ) {
            return bus->getId() == chainorbusid;
        } );
        return ( busIt != buses.end() ) ? busIt->get() : nullptr;
    }

    static IChain* _getChainOrBus( std::list<std::unique_ptr<Chain>>& chains, std::list<std::unique_ptr<Bus>>& buses, const uint32_t chainorbusid )
    {
        auto pChain = _getChain( chains, chainorbusid );
        return pChain ? dynamic_cast<IChain*>( pChain ) : dynamic_cast<IChain*>( _getBus( buses, chainorbusid ) );
    }

    //
    // Class DesignImpl: This class contains private implementation details of main Design class.
    //
    class DesignImpl
    {
    public:
        DesignImpl( uint32_t nIn, uint32_t nOut );
        ~DesignImpl() = default;

        void prepareToPlay( uint32_t sampleRate, uint32_t blockSize );

        void process( Buffer<float>& Buffer );

        bool setDesignInput( uint32_t objid, bool Force );
        bool setDesignOutput( uint32_t objid, bool Force );

        void clearDesignInput( bool Force );
        void clearDesignOutput( bool Force );

        void setDefaults();
        void clear();

        bool patch( objectMeta& srcmeta, const uint32_t srcch, objectMeta& dstmeta, const uint32_t dstch );
        bool patch( uint32_t srcid, uint32_t srcch, uint32_t dstid, uint32_t dstch );
        bool unpatch( uint32_t srcid, uint32_t srcch, uint32_t dstid, uint32_t dstch );

        bool insertObject( std::unique_ptr<Object> obj, uint32_t sampleRate, uint32_t blockSize );
        bool insertObject( Object& obj, uint32_t sampleRate, uint32_t blockSize );
        bool removeObject( objectMeta* objMeta );

        uint32_t                insertDesign( std::shared_ptr<Design> obj );
        std::shared_ptr<Design> getDesign( uint32_t id );

        inline void markDirty() { _dirty.store( true ); }
        inline bool isBypassed() const { return _bypass.load(); }
        inline void setBypass( const bool bypass )
        {
            _bypass.store( bypass );
            bypassGain.store( bypass ? 0.f : 1.f );
        }

        Chain* createChain( bool Stereo );
        Bus*   createBus();

        inline Object*        getObject( uint32_t ID ) { return _getObject( _objects, ID ); }
        inline objectMeta*    getObjectMeta( uint32_t ID ) { return _getObjectMeta( _objects, ID ); }
        inline Buffer<float>* getObjectBuffer( uint32_t ID ) { return _getObjectBuffer( _objects, ID ); }

        inline Chain*  getChain( uint32_t ID ) { return _getChain( _chains, ID ); }
        inline Bus*    getBus( uint32_t ID ) { return _getBus( _buses, ID ); }
        inline IChain* getChainOrBus( uint32_t ID ) { return _getChainOrBus( _chains, _buses, ID ); }

        inline bool exists( uint32_t ID ) { return ( nullptr != getObject( ID ) ); }

    private:
        bool isCyclic();
        void topologicalSort();
        void topologicalSortUtil( objectMeta& meta );

        bool checkCycles( objectMeta& meta );

        bool handleRemove( uint32_t id );
        void processRemoveQueue();

        inline void handleClearDesign();
        inline void handleClearDesignInput();
        inline void handleClearDesignOutput();

    public:
        MeterEngine _meters;

    private:
        std::list<std::unique_ptr<Object>> _objectPointers;
        std::vector<objectMeta*>           _dfs;
        SPSCQUeue<uint32_t, 16>            _removeQueue;
        AudIO::Buffer<float>               _metersbuff; // empty buffer for meters

        std::unordered_map<uint32_t, objectMeta> _objects;
        std::list<std::unique_ptr<Chain>>        _chains;
        std::list<std::unique_ptr<Bus>>          _buses;
        std::list<std::shared_ptr<Design>>       _designs;

        uint32_t _inputCount{ 0 };
        uint32_t _outputCount{ 0 };
        uint32_t _dfsIndex{ 0 };
        uint32_t _sampleRate;
        uint32_t _blockSize;

        // Ramped parameters...
        RampedParameter<float> bypassGain{ 1.0f };
        RampedParameter<float> designInputGain{ 0.0f };
        RampedParameter<float> designOutputGain{ 0.0f };

        // Atomic operations...
        std::atomic<bool> _dirty{ false };
        std::atomic<bool> _bypass{ true };
        std::atomic<bool> _running{ false };

        std::atomic<AudIO::Buffer<float>*> _inputObjBuffer{ nullptr };
        std::atomic<AudIO::Buffer<float>*> _outputObjBuffer{ nullptr };

        std::atomic<bool> _clearRequest{ false };
        std::atomic<bool> _clearDesignInput{ false };
        std::atomic<bool> _clearDesignOutput{ false };
    };

} // Namespace AudIO
