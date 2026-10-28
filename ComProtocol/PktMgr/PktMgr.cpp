/*
 * Copyright (c) MUSIC Tribe
 * This file is part of ComProtocol.
 *
 * ComProtocol is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * ComProtocol is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with ComProtocol. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "PktMgr.hpp"

#ifdef KICK_MCU
    #include <Hardware.hpp>
    #include <TaskHdl.hpp>
    #include <ComHdl.hpp>
#endif

void CPktMgr::Init( void )
{
    m_ui8FrameSize       = 0;
    m_ui8RxCommand       = 0;
    m_ui8TxCommand       = 0;
    m_ui8RxFifoWritePos  = 0;
    m_ui8RxFifoReadPos   = 0;
    m_boFrameRcvComplete = FALSE;
    m_enPktDir           = EN_PKT_REQUEST;

#ifdef PKT_HEADER_AVAILABLE
    m_enFrmState = EN_FRM_STATE_INIT_1;
#else
    m_enFrmState = EN_FRM_STATE_TYPE;
#endif
}

void CPktMgr::ClearReceiver( void )
{
    m_ui8RxFifoWritePos  = 0;
    m_ui8RxFifoReadPos   = 0;
    m_enPktDir           = EN_PKT_REQUEST;
    m_boFrameRcvComplete = FALSE;
    m_enFrmState         = EN_FRM_STATE_INIT_1;
}

void CPktMgr::PushByteToRxFifo( uint8_t f_ui8Data )
{
    static std::vector<char> ui8TempReceiveData;
    bool                     boRcvdCompleteMsg = FALSE;

    switch( m_enFrmState )
    {
        case EN_FRM_STATE_INIT_1:
            ui8TempReceiveData.push_back( f_ui8Data );
            if( f_ui8Data == 'M' )
            {
                m_enFrmState = EN_FRM_STATE_INIT_2;
            }

            break;
        case EN_FRM_STATE_INIT_2:
            ui8TempReceiveData.push_back( f_ui8Data );
            if( f_ui8Data == 'T' )
            {
                m_enFrmState = EN_FRM_STATE_TYPE;
            }

            break;
        case EN_FRM_STATE_TYPE:

            m_enPktType  = (enumPacketType)f_ui8Data;
            m_enFrmState = EN_FRM_STATE_DIR;
            ui8TempReceiveData.push_back( f_ui8Data );
            break;
        case EN_FRM_STATE_DIR:
            ui8TempReceiveData.push_back( f_ui8Data );
            m_enPktDir   = (enumPacketDir)f_ui8Data;
            m_enFrmState = EN_FRM_STATE_ID;
            break;
        case EN_FRM_STATE_ID:
            ui8TempReceiveData.push_back( f_ui8Data );
            m_ui8RxCommand = f_ui8Data;
            m_enFrmState   = EN_FRM_STATE_SIZE;
            break;
        case EN_FRM_STATE_SIZE:
            ui8TempReceiveData.push_back( f_ui8Data );
            m_ui8FrameSize = ( f_ui8Data );
            m_enFrmState   = EN_FRM_STATE_DATA;

            if( m_ui8FrameSize <= PKTMGR_CRC_BYTE )
            {
                m_enFrmState = EN_FRM_STATE_CRC;
            }
            break;

        case EN_FRM_STATE_DATA:
            ui8TempReceiveData.push_back( f_ui8Data );
            if( m_ui8RxFifoWritePos < ( m_ui8FrameSize - 1 ) )
            {
                m_ptrui8RxFifo[m_ui8RxFifoWritePos] = f_ui8Data;
                m_ui8RxFifoWritePos++;

                if( m_ui8RxFifoWritePos == ( m_ui8FrameSize - 1 ) )
                {
                    m_enFrmState = EN_FRM_STATE_CRC;
                }
            }
            break;
        case EN_FRM_STATE_CRC:
            uint8_t ui8CrcTemp = 0;
            ui8CrcTemp         = CrcGen( ui8TempReceiveData );
            if( ui8CrcTemp == f_ui8Data )
            {
                boRcvdCompleteMsg = TRUE;
            }
            ui8TempReceiveData.clear();
            break;
    }

    if( boRcvdCompleteMsg )
    {
        m_boFrameRcvComplete = TRUE;
    }
}

uint8_t CPktMgr::PopUint8FromRxFifo( void )
{
    return m_ptrui8RxFifo[m_ui8RxFifoReadPos++];
}

uint8_t CPktMgr::PopUint8FromTxFifo( void )
{
    return m_ptrui8TxFifo[m_ui8TxFifoReadPos++];
}

void CPktMgr::PushUint8ToTxFifo( uint8_t f_ui8Data )
{
    m_ptrui8TxFifo[m_ui8TxFifoWritePos++] = f_ui8Data;
}

uint8_t CPktMgr::CrcGen( std::vector<char>& f_txData )
{
    uint8_t ui8CrcValue = 0xff;
    uint8_t i, j;
    uint8_t len = f_txData.size();

    for( i = 0; i < len; i++ )
    {
        ui8CrcValue ^= f_txData[i];
        for( j = 0; j < 8; j++ )
        {
            if( ( ui8CrcValue & 0x80 ) != 0 )
                ui8CrcValue = (uint8_t)( ( ui8CrcValue << 1 ) ^ 0x31 );
            else
                ui8CrcValue <<= 1;
        }
    }
    return ui8CrcValue;
}

void CPktMgr::Reset( CPktMgr::HWMsg_t& f_HwMsg )
{
    f_HwMsg.pktType = (enumPacketType)0;
    f_HwMsg.pktDir  = (enumPacketDir)0;
    f_HwMsg.pktCmd  = 0;
    f_HwMsg.data.clear();
}

void CPktMgr::BuildAndTransmitPacket( CPktMgr::HWMsg_t& f_HwMsg, std::vector<char>& f_txData )
{
    uint8_t ui8TxBufferIndex = 0;
    int     sizeValue        = (int)f_HwMsg.data.size();
    int     txIndex          = 0;

    f_txData.push_back( 'M' );
    f_txData.push_back( 'T' );
    f_txData.push_back( f_HwMsg.pktType );
    f_txData.push_back( f_HwMsg.pktDir );
    f_txData.push_back( f_HwMsg.pktCmd );
    f_txData.push_back( sizeValue + PKTMGR_CRC_BYTE );

    while( sizeValue )
    {
        f_txData.push_back( f_HwMsg.data[txIndex] );
        sizeValue--;
        txIndex++;
    }

    f_txData.push_back( CrcGen( f_txData ) );
    //f_txData.push_back('\n');

    m_ui8TxCommand = static_cast<uint8_t>(f_HwMsg.pktCmd);
}
