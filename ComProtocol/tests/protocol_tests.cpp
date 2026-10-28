// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PktMgr.hpp"
#include <iostream>
int main()
{
    int checks=0,failures=0;
    auto check=[&](bool ok,const char* name){++checks;if(!ok){++failures;std::cerr<<"FAIL: "<<name<<'\n';}};
    uint8_t rx[32]{},tx[32]{};
    CPktMgr packet(CPktMgr::EN_COM_CHANNEL_USB,rx,32,tx,32,100);packet.Init();
    CPktMgr::HWMsg_t message;message.pktType=CPktMgr::EN_PKT_TYPE_MIDI;message.pktCmd=7;message.data={0x90,60,100};
    std::vector<char> bytes;packet.BuildAndTransmitPacket(message,bytes);
    check(packet.GetTxCommand()==7,"transmitted command comes from the message");
    for(unsigned char byte:bytes)packet.PushByteToRxFifo(byte);
    check(packet.IsFrameRcvComplete() && packet.GetRxCommand()==7 && packet.GetRxPktType()==CPktMgr::EN_PKT_TYPE_MIDI,"MIDI packet roundtrip");
    check(packet.PopUint8FromRxFifo()==0x90 && packet.PopUint8FromRxFifo()==60 && packet.PopUint8FromRxFifo()==100,"payload bytes preserved");
    packet.ClearReceiver();bytes.back()^=1;
    for(unsigned char byte:bytes)packet.PushByteToRxFifo(byte);
    check(!packet.IsFrameRcvComplete(),"corrupt CRC is rejected");
    packet.ClearReceiver();message.data.clear();bytes.clear();packet.BuildAndTransmitPacket(message,bytes);
    for(unsigned char byte:bytes)packet.PushByteToRxFifo(byte);
    check(packet.IsFrameRcvComplete(),"empty command packet roundtrip");
    std::cout<<checks-failures<<'/'<<checks<<" checks passed\n";
    return failures?1:0;
}
