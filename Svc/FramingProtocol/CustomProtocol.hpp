#ifndef CUSTOM_PROTOCOL_HPP
#define CUSTOM_PROTOCOL_HPP

#include <Svc/FramingProtocol/FramingProtocol.hpp>
#include <Svc/FramingProtocol/DeframingProtocol.hpp>
//#include <Svc/FramingProtocol/FramingProtocolInterface.hpp>
#include <Fw/Ports/Node/NodeEnumAc.hpp>

namespace Svc {
  // Definitions for the F Prime frame header
  namespace CgFrameHeader {

    //! Token type for F Prime frame header
    typedef U32 TokenType;
    typedef U16 HalfTokenType;

    enum {
      //! Header size for F Prime frame header
      SIZE = sizeof(TokenType) * 2
    };

    //! The start word for F Prime framing
    const TokenType START_WORD = static_cast<TokenType>(0xdeadbeef);
    //! The destination and source of each packetf
 /*    enum Node{
      MPU = 0xaa,
      MCU = 0xbb,
      FPGA = 0xcc,
      GPU = 0xdd,
      OTV = 0xee,
      GDS = 0xff
    }; */

  }

  class CgFraming: public Svc::FramingProtocol {

    public:

      //! Constructor
      CgFraming();

      void setup(Svc::FramingProtocolInterface& interface) ;

      //! Frame with packet ID
      void frame(
          const U8* const data, //!< The data
          const U8 packetID,
          const U32 size, //!< The data size in bytes
          Fw::ComPacket::ComPacketType packet_type //!< The packet type
      );

      //! Implements the frame method
      void frame(
          const U8* const data, //!< The data
          const U32 size, //!< The data size in bytes
          Fw::ComPacket::ComPacketType packet_type //!< The packet type
      ) override;

      //CgFrameHeader::Node dest_node;
      Components::Node dest_node;

      void set_node(Components::Node node);
      void frame_ack(const U8 packetID);

    private:
      U8 packet_id = 0;
      U32 header_size;
  };

  class CgDeframing : public Svc::DeframingProtocol {
    public:

      //! Constructor
      CgDeframing();

      void setup(Svc::DeframingProtocolInterface& interface) ;

      //! Validates data against the stored hash value
      //! 1. Computes the hash value V of bytes [0,size-1] in the circular buffer
      //! 2. Compares V against bytes [size, size + HASH_DIGEST_LENGTH - 1] of
      //!    the circular buffer, which are expected to be the stored hash value.
      bool validate(
          Types::CircularBuffer& buffer, //!< The circular buffer
          U32 size //!< The data size in bytes
      );

      //! Implements the deframe method
      //! \return Status
      DeframingStatus deframe(
          Types::CircularBuffer& buffer, //!< The circular buffer
          U32& needed //!< The number of bytes needed, updated by the caller
      ) override;

      U8 get_packet_id();


      //! Implements the frame method
      void send_ack(
          const U8 packetID //!< The packetID to acknowledge
      );

      U32 header_size;
      U8 packet_id = 0;

  };

  U8 get_packetID();
}
#endif