#include <Fw/Types/Serializable.hpp>
#include "AckPacket.hpp"
#include <Fw/Types/Assert.hpp>
#include <iostream>

namespace Fw {

    AckPacket::AckPacket() : m_numEntries(0) {
        this->m_type = FW_PACKET_ACK;
        this->m_ackBuffer.resetSer();      
    }

    AckPacket::~AckPacket() {
    }

    Fw::ComBuffer& AckPacket::getBuffer() {
        return this->m_ackBuffer;
    }
   
    SerializeStatus AckPacket::serialize(SerializeBufferBase& buffer) const {
        Fw::SerializeStatus status;
        status = buffer.serialize(static_cast<FwPacketDescriptorType>(this->m_type));
        if (status != Fw::FW_SERIALIZE_OK) {
            return status;
        }
        status = buffer.serialize(this->packetID);
        return status;
    }
    
    SerializeStatus AckPacket::deserialize(SerializeBufferBase& buffer) {
        FwPacketDescriptorType type;
        U8 pktID;
        SerializeStatus status = buffer.deserialize(type);
        if (status != Fw::FW_SERIALIZE_OK) {
            return status;
        }
        status = buffer.deserialize(pktID);
        if (status != Fw::FW_SERIALIZE_OK) {
            
            return status;
        }
        if(type != Fw::ComPacket::ComPacketType::FW_PACKET_ACK){
            return FW_DESERIALIZE_TYPE_MISMATCH;
        }
        this->packetID = pktID;
        return status;
    }

    void AckPacket::setPacketID(U8 pktID){
        this->m_ackBuffer.resetSer();
        this->packetID = pktID;
        this->m_ackBuffer.serialize(static_cast<FwPacketDescriptorType>(this->m_type));
        this->m_ackBuffer.serialize(this->packetID);
    }

    U8 AckPacket::getPacketID(){
        return this->packetID;
    }

} /* namespace Fw */
