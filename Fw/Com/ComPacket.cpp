/*
 * ComPacket.cpp
 *
 *  Created on: May 24, 2014
 *      Author: Timothy Canham
 */

#include <Fw/Com/ComPacket.hpp>

namespace Fw {

    ComPacket::ComPacket() : m_type(FW_PACKET_UNKNOWN) {
    }

    ComPacket::~ComPacket() {
    }

    /* SerializeStatus ComPacket::serializeBase(SerializeBufferBase& buffer) const {
        return buffer.serialize(static_cast<FwPacketDescriptorType>(this->m_type));
    } */
    SerializeStatus ComPacket::serializeBase(SerializeBufferBase& buffer) const {
        SerializeStatus status ;
        status = buffer.serialize(static_cast<FwPacketDescriptorType>(this->dest));
        status = buffer.serialize(static_cast<FwPacketDescriptorType>(this->m_type));
        return status;
    }

    SerializeStatus ComPacket::deserializeBase(SerializeBufferBase& buffer) {
        FwPacketDescriptorType serVal;
        SerializeStatus stat;
        Components::Node destNode;
        // TODO At least for now, we do not need to deserialize the destNode, as it is handled by the CgDeframer
        // the issue is that we need to deserialize the destNode in the CgFramer to know where to send it
        stat = buffer.deserialize(destNode);
        if (FW_SERIALIZE_OK == stat) {
            this->dest = destNode;
        }
        stat = buffer.deserialize(serVal);
        if (FW_SERIALIZE_OK == stat) {
            this->m_type = static_cast<ComPacketType>(serVal);
        }
        return stat;
    }

    SerializeStatus ComPacket::deserializeWithoutDest(SerializeBufferBase& buffer) {
        FwPacketDescriptorType serVal;
        SerializeStatus stat;
        
        stat = buffer.deserialize(serVal);
        if (FW_SERIALIZE_OK == stat) {
            this->m_type = static_cast<ComPacketType>(serVal);
        }
        return stat;
    }


    SerializeStatus ComPacket::deserializeDestNode(SerializeBufferBase& buffer) {
        SerializeStatus stat;
        Components::Node destNode;
        stat = buffer.deserialize(destNode);
        if (FW_SERIALIZE_OK == stat) {
            this->dest.e = destNode.e;
        }
        return stat;
    }

    Components::Node ComPacket::getDestNode(){
        return this->dest;
    }

    void ComPacket::setDestNode(Components::Node destNode){
            this->dest.e = destNode.e;
        
    }


} /* namespace Fw */
