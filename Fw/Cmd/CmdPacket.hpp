/*
 * CmdPacket.hpp
 *
 *  Created on: May 24, 2014
 *      Author: Timothy Canham
 */

#ifndef CMDPACKET_HPP_
#define CMDPACKET_HPP_

#include <Fw/Com/ComPacket.hpp>
#include <Fw/Cmd/CmdArgBuffer.hpp>

namespace Fw {

    class CmdPacket : public ComPacket {
        public:

            CmdPacket();
            virtual ~CmdPacket();

            SerializeStatus serialize(SerializeBufferBase& buffer) const; //!< serialize contents
            SerializeStatus deserialize(SerializeBufferBase& buffer);
            FwOpcodeType getOpCode() const;
            CmdArgBuffer& getArgBuffer();
            void setOpcode(FwOpcodeType opcode);
            void setArgBuffer(CmdArgBuffer argbuf);

        protected:
            FwOpcodeType m_opcode;
            CmdArgBuffer m_argBuffer;
    };

} /* namespace Fw */

#endif /* CMDPACKET_HPP_ */
