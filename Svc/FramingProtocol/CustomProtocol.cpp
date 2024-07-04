#include "CustomProtocol.hpp"
#include <Fw/Types/String.hpp>
#include "FpConfig.h"
#include "Utils/Hash/Hash.hpp"
//#include "../Components/AckTracker/AckPacket.hpp"
#include <iostream>


namespace Svc {
#define USE_PACKETID 

    static U8 deframed_packetID = 0;

    CgFraming::CgFraming(): Svc::FramingProtocol(){
#ifdef USE_PACKETID
            std::cout << "[CgFraming] Using PacketID in Framing" << std::endl;
            this->header_size = CgFrameHeader::SIZE + 1; // we add one to take into account the packetID
#else
            this->header_size = CgFrameHeader::SIZE;
#endif
    }
    
    CgDeframing::CgDeframing(): Svc::DeframingProtocol(){
#ifdef USE_PACKETID
            this->header_size = CgFrameHeader::SIZE + 1; // we add one to take into account the packetID
#else
            this->header_size = CgFrameHeader::SIZE;
#endif
      
    }

    void CgFraming::setup(Svc::FramingProtocolInterface& interface) {
        FW_ASSERT(m_interface == nullptr);
        m_interface = &interface;
    }

    void CgFraming::set_node(Components::Node node){
        this->dest_node = node;
    }

    void CgFraming::frame(const U8* const data, const U8 packetID, const U32 size, Fw::ComPacket::ComPacketType packet_type) {
        this->packet_id = packetID;
        this->frame(data, size, packet_type);
    }

    void CgFraming::frame(const U8* const data, const U32 size, Fw::ComPacket::ComPacketType packet_type) {
        //Fw::ComPacket::ComPacketType packet_type_from_data = packet_type;
        FW_ASSERT(data != nullptr);
        FW_ASSERT(m_interface != nullptr);
        // Use of I32 size is explicit as ComPacketType will be specifically serialized as an I32
        CgFrameHeader::HalfTokenType real_data_size = static_cast<CgFrameHeader::HalfTokenType>
                                                        (size + ((packet_type != Fw::ComPacket::FW_PACKET_UNKNOWN) ? sizeof(I32) : 0));
        CgFrameHeader::HalfTokenType total = static_cast<CgFrameHeader::HalfTokenType>
                                                        (real_data_size + this->header_size + HASH_DIGEST_LENGTH);
        CgFrameHeader::HalfTokenType metadata = (Components::Node::MPU<<8) | (Components::Node::MPU);

        Fw::Buffer buffer = m_interface->allocate(total);
        Fw::SerializeBufferBase& serializer = buffer.getSerializeRepr();
        Utils::HashBuffer hash;

        // Serialize data
        Fw::SerializeStatus status;
        status = serializer.serialize(CgFrameHeader::START_WORD);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

        status = serializer.serialize(metadata);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        
        status = serializer.serialize(real_data_size);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

#ifdef USE_PACKETID
            status = serializer.serialize(this->packet_id);
            FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
#endif
        
        // Serialize packet type if supplied, otherwise it *must* be present in the data
        if (packet_type != Fw::ComPacket::FW_PACKET_UNKNOWN) {
            status = serializer.serialize(static_cast<I32>(packet_type)); // I32 used for enum storage
            FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        }
        
        status = serializer.serialize(data, size, true);  // Serialize without length
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

        // Calculate and add transmission hash
        Utils::Hash::hash(buffer.getData(), total - HASH_DIGEST_LENGTH, hash);
        status = serializer.serialize(hash.getBuffAddr(), HASH_DIGEST_LENGTH, true);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        
        buffer.setSize(total);
        
        m_interface->send(buffer);
    }

    void CgFraming::frame_ack(const U8 packetID) {
        std::cout << "[CgFraming] Sending ACK for packetID " << static_cast<unsigned int>(packetID) <<std::endl;
        FW_ASSERT(m_interface != nullptr);
        CgFrameHeader::HalfTokenType total = static_cast<CgFrameHeader::HalfTokenType>
                                                ( this->header_size + sizeof(I32) + HASH_DIGEST_LENGTH);
        CgFrameHeader::HalfTokenType metadata = (Components::Node::MPU<<8) | (Components::Node::GDS);
        CgFrameHeader::HalfTokenType size = sizeof(I32);
        
        Fw::Buffer buffer = m_interface->allocate(total);
        Fw::SerializeBufferBase& serializer = buffer.getSerializeRepr();
        Utils::HashBuffer hash;

        Fw::SerializeStatus status;
        status = serializer.serialize(CgFrameHeader::START_WORD);                   // 4B
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

        status = serializer.serialize(metadata);                                    // 2B
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        
        status = serializer.serialize(size);                                        // 2B
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

        status = serializer.serialize(packetID);                                    // 1B
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        
        status = serializer.serialize(Fw::ComPacket::ComPacketType::FW_PACKET_ACK); // 4B
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        
        // Calculate and add transmission hash
        Utils::Hash::hash(buffer.getData(), total - HASH_DIGEST_LENGTH, hash);
        status = serializer.serialize(hash.getBuffAddr(), HASH_DIGEST_LENGTH, true);// 4B
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        
        buffer.setSize(total);
        
        m_interface->send(buffer, true);
    }


/*
            CgDeframing
*/

    bool CgDeframing::validate(Types::CircularBuffer& ring, U32 size) {
        Utils::Hash hash;
        Utils::HashBuffer hashBuffer;
        // Initialize the checksum and loop through all bytes calculating it
        hash.init();
        for (U32 i = 0; i < size; i++) {
            U8 byte;
            const Fw::SerializeStatus status = ring.peek(byte, i);
            FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
            hash.update(&byte, 1);
        }
        hash.final(hashBuffer);
        // Now loop through the hash digest bytes and check for equality
        for (U32 i = 0; i < HASH_DIGEST_LENGTH; i++) {
            U8 calc = static_cast<char>(hashBuffer.getBuffAddr()[i]);
            U8 sent = 0;
            const Fw::SerializeStatus status = ring.peek(sent, size + i);
            FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
            if (calc != sent) {
                return false;
            }
        }
        return true;
    }

    void CgDeframing::setup(Svc::DeframingProtocolInterface& interface) {
        FW_ASSERT(m_interface == nullptr);
        m_interface = &interface;
    }

    Svc::DeframingProtocol::DeframingStatus CgDeframing::deframe(Types::CircularBuffer& ring, U32& needed) {
        
        CgFrameHeader::TokenType start = 0;
        CgFrameHeader::TokenType size_dest = 0;
        CgFrameHeader::HalfTokenType size = 0;
        CgFrameHeader::HalfTokenType destination = 0;
        //CgFrameHeader::HalfTokenType source = 0;


        FW_ASSERT(m_interface != nullptr);
        // Check for header or ask for more data
        if (ring.get_allocated_size() < this->header_size) {
            needed = this->header_size;
            return Svc::DeframingProtocol::DEFRAMING_MORE_NEEDED;
        }
        
        // Read start value from header
        Fw::SerializeStatus status = ring.peek(start, 0);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        if (start != CgFrameHeader::START_WORD) {
            // Start word must be valid
            return Svc::DeframingProtocol::DEFRAMING_INVALID_FORMAT;
        }

        // Read size from header
        status = ring.peek(size_dest, sizeof(CgFrameHeader::TokenType));
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        
        
        // Read size from header
        //status = ring.peek(this->packet_id, sizeof(CgFrameHeader::TokenType) * 2);
        status = ring.peek(deframed_packetID, sizeof(CgFrameHeader::TokenType) * 2);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        //std::cout << "[ProtocolDeframe] Received packetID "<< static_cast<unsigned int>(this->packet_id) <<std::endl;
        //source    = static_cast<CgFrameHeader::HalfTokenType>((size_dest & 0xFF000000)>>24);
        destination = static_cast<CgFrameHeader::HalfTokenType>((size_dest & 0x00FF0000)>>16);
        size        = static_cast<CgFrameHeader::HalfTokenType>((size_dest & 0x0000FFFF));
        
        const U16 maxU16 = std::numeric_limits<U16>::max();

        if (size > maxU16 - (this->header_size + HASH_DIGEST_LENGTH)) {
            // Size is too large to process: needed would overflow
            return Svc::DeframingProtocol::DEFRAMING_INVALID_SIZE;
        }
        if (destination != Components::Node::MPU) {
            // Packet is not meant to the MPU, reject for now
            return Svc::DeframingProtocol::DEFRAMING_WRONG_DESTINATION;
        }

        needed = (this->header_size + size + HASH_DIGEST_LENGTH);
        // Check frame size
        const U32 frameSize = size + this->header_size + HASH_DIGEST_LENGTH;
        if (frameSize > ring.get_capacity()) {
            // Frame size is too large for ring buffer
            return Svc::DeframingProtocol::DEFRAMING_INVALID_SIZE;
        }
        // Check for enough data to deserialize everything;
        // otherwise break and wait for more.
        else if (ring.get_allocated_size() < needed) {
            return Svc::DeframingProtocol::DEFRAMING_MORE_NEEDED;
        }
        // Check the checksum
        if (not this->validate(ring, needed - HASH_DIGEST_LENGTH)) {
            return Svc::DeframingProtocol::DEFRAMING_INVALID_CHECKSUM;
        }
        Fw::Buffer buffer = m_interface->allocate(size);
        // Some allocators may return buffers larger than requested.
        // That causes issues in routing; adjust size.
        FW_ASSERT(buffer.getSize() >= size);
        buffer.setSize(size);
        status = ring.peek(buffer.getData(), size, this->header_size);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        m_interface->route(buffer);
        
        return Svc::DeframingProtocol::DEFRAMING_STATUS_SUCCESS;
    }
    
    // packetID of the deframed packet !! 
    U8 get_packetID(){
        //return this->packet_id;
        return deframed_packetID;
    }
}