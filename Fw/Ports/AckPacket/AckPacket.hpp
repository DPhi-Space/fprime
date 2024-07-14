#ifndef ACKPACKET_HPP_
#define ACKPACKET_HPP_

#include <Fw/Com/ComPacket.hpp>
#include <Fw/Com/ComBuffer.hpp>
#include <Fw/Time/Time.hpp>

namespace Fw {

    class AckPacket : public ComPacket {
        public:

            //! Constructor
            AckPacket();
            //! Destructor
            virtual ~AckPacket();

            //! Serialize the packet before sending. For use internally in software. To send to the ground, use getBuffer() below.
            SerializeStatus serialize(SerializeBufferBase& buffer) const; //!< serialize contents
            //! Deserialize the packet.
            SerializeStatus deserialize(SerializeBufferBase& buffer);
            //! get buffer to send to the ground
            Fw::ComBuffer& getBuffer();

            void setPacketID(U8 packetID);
            U8 getPacketID();
        PRIVATE:
            ComBuffer m_ackBuffer; //!< serialized data
            U8 packetID;
            NATIVE_UINT_TYPE m_numEntries; //!< number of entries stored during addValue()
    };

} /* namespace Fw */

#endif /* ACKPACKET_HPP_ */
