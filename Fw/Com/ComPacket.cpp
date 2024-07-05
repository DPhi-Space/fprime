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
        // At least for now, we do not need to deserialize the destNode, as it is handled by the CgDeframer
        //stat = buffer.deserialize(destNode);
        stat = buffer.deserialize(serVal);
        if (FW_SERIALIZE_OK == stat) {
            this->m_type = static_cast<ComPacketType>(serVal);
        }
        return stat;
    }


} /* namespace Fw */
