#include <Fw/Types/Serializable.hpp>
#include "RetPacket.hpp"
#include <Fw/Types/Assert.hpp>
#include <iostream>

namespace Fw {

    // Empty RetPacket, to use use deserialization
    RetPacket::RetPacket()
    {
        this->data.resetSer();        
    }

    // For RetPacket ERRORS
    RetPacket::RetPacket(Fw::ComPacket::ComPacketType type, U32 seq, Code err_code, Components::Node destination)
    {
        FW_ASSERT(type == ComPacket::FW_PACKET_RET_ERR);
        this->m_type = type;
        this->data.resetSer();
        this->code = err_code;
        this->dest = destination;
        this->cmdSeq = static_cast<U16>(seq);
    }

    // For RetPackets OK
    RetPacket::RetPacket(Fw::ComPacket::ComPacketType type, U32 seq, Components::Node destination)
    {
        FW_ASSERT(type == ComPacket::FW_PACKET_RET_OK);
        this->m_type = type;
        this->dest = destination;
        this->data.resetSer();
        this->cmdSeq = static_cast<U16>(seq);
    }

    RetPacket::~RetPacket() {}


    SerializeStatus RetPacket::serialize(SerializeBufferBase& buffer) const 
    {
        // serialize the destination node
        SerializeStatus stat = buffer.serialize(this->dest.e);
        
        if (stat != Fw::FW_SERIALIZE_OK) {
            return stat;
        }

        // serialize the packet type
        stat = buffer.serialize(this->m_type);
        
        if (stat != Fw::FW_SERIALIZE_OK) {
            return stat;
        }

        stat = buffer.serialize(this->cmdSeq);

        if (stat != Fw::FW_SERIALIZE_OK) {
            return stat;
        }

        if (this->m_type == Fw::ComPacket::FW_PACKET_RET_OK)
        {
            stat = buffer.serialize(this->data_size);
            //stat = buffer.serialize(this->data);
        }
        else
        {
            stat = buffer.serialize(this->code);
        }
        return stat;
    }

    U16 RetPacket::getCmdSeq()
    {
        return this->cmdSeq;
    }

    Fw::ComPacket::ComPacketType RetPacket::getType()
    {
        return this->m_type;
    }

    SerializeStatus RetPacket::deserialize(SerializeBufferBase& buffer) 
    {
        I32 type;
        U16 cmdSeqId;
        Components::Node destination;
        
        SerializeStatus status = buffer.deserialize(destination);
        if (status != Fw::FW_SERIALIZE_OK) {
            return status;
        }
        
        status = buffer.deserialize(type);
        if (status != Fw::FW_SERIALIZE_OK) {
            return status;
        }
        status = buffer.deserialize(cmdSeqId);
        if (status != Fw::FW_SERIALIZE_OK) {
            return status;
        }
        
        this->dest = destination;
        this->m_type = static_cast<Fw::ComPacket::ComPacketType>(type);
        this->cmdSeq = cmdSeqId;
        
        return status;
    }


    Fw::ComBuffer& RetPacket::getBuffer() 
    {
        FW_ASSERT(this->m_type != ComPacket::FW_PACKET_RET_OK);
        return this->data;
    }
   



} /* namespace Fw */
