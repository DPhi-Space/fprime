#ifndef RETPACKET_HPP_
#define RETPACKET_HPP_

#include <Fw/Com/ComPacket.hpp>
#include <Fw/Com/ComBuffer.hpp>
#include <Fw/Time/Time.hpp>

namespace Fw {

    class RetPacket : public ComPacket {
    public:

        typedef enum : U8 {
            ERROR_CMD_FAILED,
            ERROR_BUSY,
        } Code;

        //! Constructor
        RetPacket();
        RetPacket(Fw::ComPacket::ComPacketType type, U32 seq, Code err_code, Components::Node destination);
        RetPacket(ComPacket::ComPacketType type, U32 cmdSeq, Components::Node destination);
        //! Destructor
        virtual ~RetPacket();

        U16 getCmdSeq();
        Fw::ComBuffer& getBuffer();
        Fw::ComPacket::ComPacketType getType();

        SerializeStatus serialize(SerializeBufferBase& buffer) const; //!< serialize contents
        SerializeStatus deserialize(SerializeBufferBase& buffer);

    PRIVATE:
        ComBuffer data; //!< serialized data
        U16 data_size = 0;
        Code code;
        U16 cmdSeq;
        NATIVE_UINT_TYPE m_numEntries; //!< number of entries stored during addValue()
    };

} /* namespace Fw */

#endif /* RETPACKET_HPP_ */
