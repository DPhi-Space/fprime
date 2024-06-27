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


            // when a component within fprime sends a cmdpacket to 
            // the cmd dispatcher, we will let it manage the cmd seq id
            // as we are not aware of the current cmd seq id counter
            typedef enum {
                INTERNAL_CMD_CONTEXT = 0xFFFF,
                EXTERNAL_CMD_CONTEXT = 0xFAFA,
            } CmdContext;


            SerializeStatus serialize(SerializeBufferBase& buffer) const; //!< serialize contents
            SerializeStatus deserialize(SerializeBufferBase& buffer);
            FwOpcodeType getOpCode() const;
            CmdArgBuffer& getArgBuffer();
            void setOpcode(FwOpcodeType opcode);
            void setArgBuffer(CmdArgBuffer argbuf);

        protected:
            FwOpcodeType m_opcode;
            CmdArgBuffer m_argBuffer;
            U32 cmdSeq;
    };

} /* namespace Fw */

#endif /* CMDPACKET_HPP_ */
