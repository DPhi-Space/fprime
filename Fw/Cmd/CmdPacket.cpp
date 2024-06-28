/*
 * CmdPacket.cpp
 *
 *  Created on: May 24, 2014
 *      Author: Timothy Canham
 */

#include <Fw/Cmd/CmdPacket.hpp>
#include <Fw/Types/Assert.hpp>
#include <cstdio>

namespace Fw {

    CmdPacket::CmdPacket() : m_opcode(0) {
        this->m_type = FW_PACKET_COMMAND;
    }

    CmdPacket::~CmdPacket() {
    }

    SerializeStatus CmdPacket::serialize(SerializeBufferBase& buffer) const {

        // Shouldn't be called
     /*    FW_ASSERT(0);
        return FW_SERIALIZE_OK; // for compiler */

        // serialize the packet type
        SerializeStatus stat = buffer.serialize(this->m_type);
        if (stat != Fw::FW_SERIALIZE_OK) {
            return stat;
        }
        
        // serialize the number of packets
        stat = buffer.serialize(this->m_opcode);

        if (stat != Fw::FW_SERIALIZE_OK) {
            return stat;
        }
        // Serialize the ComBuffer
        return buffer.serialize(this->m_argBuffer.getBuffAddr(),m_argBuffer.getBuffLength(),true);

    }

    SerializeStatus CmdPacket::deserialize(SerializeBufferBase& buffer) {

        SerializeStatus stat = ComPacket::deserializeBase(buffer);
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }

        // double check packet type
        if (this->m_type != FW_PACKET_COMMAND) {
            return FW_DESERIALIZE_TYPE_MISMATCH;
        }

        stat = buffer.deserialize(this->m_opcode);
        
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }

        // separate the cmdSeq from the opcode (each 2B)
        this->cmdSeq = static_cast<U16>((this->m_opcode & 0xFFFF0000) >> 16);
        this->m_opcode = (this->m_opcode & 0x0000FFFF);

        // if non-empty, copy data
        if (buffer.getBuffLeft()) {
            // copy the serialized arguments to the buffer
            stat = buffer.copyRaw(this->m_argBuffer,buffer.getBuffLeft());
        }

        return stat;
    }

    FwOpcodeType CmdPacket::getOpCode() const {
        return this->m_opcode;
    }

    CmdArgBuffer& CmdPacket::getArgBuffer() {
        return this->m_argBuffer;
    }

    void CmdPacket::setSeqId(U16 cmdSeqId){
        this->cmdSeq = cmdSeqId;
    }

    void CmdPacket::setOpcode(FwOpcodeType opcode){
        this->m_opcode = opcode;
    }

    
    void CmdPacket::setArgBuffer(CmdArgBuffer argbuf){
        this->m_argBuffer = argbuf;
    }


} /* namespace Fw */
