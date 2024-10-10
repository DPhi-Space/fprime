// ======================================================================
// \title  FileUplink.cpp
// \author bocchino
// \brief  cpp file for FileUplink component implementation class
//
// \copyright
// Copyright 2009-2016, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#include <Svc/FileUplink/FileUplink.hpp>
#include <Fw/Types/Assert.hpp>
#include <FpConfig.hpp>
#include <iostream>

namespace Svc {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  FileUplink::FileUplink(const char* const name) :
    FileUplinkComponentBase(name),
    m_receiveMode(START),
    m_lastSequenceIndex(0),
    m_filesReceived(this),
    m_packetsReceived(this),
    m_warnings(this),
    m_packetCounter(0),
    m_endPacketIndex(0)
  {

  }

  void FileUplink::init(
    const NATIVE_INT_TYPE queueDepth,
    const NATIVE_INT_TYPE instance
  )
  {
    FileUplinkComponentBase::init(queueDepth, instance);
  }

  FileUplink ::~FileUplink()
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void FileUplink::bufferSendIn_handler(
    const NATIVE_INT_TYPE portNum,
    Fw::Buffer& buffer
  )
  {
    Fw::FilePacket filePacket;
    U8 packetID = buffer.getData()[0];
    U8 source = buffer.getData()[1];

    // we shift out the packet id and source from the filepacket buffer

    Fw::Buffer fileDataBuffer(buffer.getData() + 2, buffer.getSize() - 2, buffer.getContext());

    const Fw::SerializeStatus status = filePacket.fromBuffer(fileDataBuffer);
    if (status != Fw::FW_SERIALIZE_OK) {
      this->log_WARNING_HI_DecodeError(status);
    }
    else {
      Fw::FilePacket::Type header_type = filePacket.asHeader().getType();
      switch (header_type) {
      case Fw::FilePacket::T_START:
        this->handleStartPacket(filePacket.asStartPacket(), packetID, source);
        break;
      case Fw::FilePacket::T_DATA:
        this->handleDataPacket(filePacket.asDataPacket(), packetID, source);
        break;
      case Fw::FilePacket::T_END:
        this->handleEndPacket(filePacket.asEndPacket(), packetID, source);
        break;
      case Fw::FilePacket::T_CANCEL:
        this->handleCancelPacket();
        break;
      default:
        FW_ASSERT(0);
        break;
      }
    }
    this->bufferSendOut_out(0, buffer);
  }

  void FileUplink::pingIn_handler(
    const NATIVE_INT_TYPE portNum,
    U32 key
  )
  {
    // return key
    this->pingOut_out(0, key);
  }

  // ----------------------------------------------------------------------
  // Private helper functions
  // ----------------------------------------------------------------------

  void FileUplink::handleStartPacket(const Fw::FilePacket::StartPacket& startPacket, U8 packetID, U8 source)
  {
    // Clear all event throttles in preparation for new start packet
    this->log_WARNING_HI_FileWriteError_ThrottleClear();
    this->log_WARNING_HI_InvalidReceiveMode_ThrottleClear();
    this->log_WARNING_HI_PacketOutOfBounds_ThrottleClear();
    this->log_WARNING_HI_PacketOutOfOrder_ThrottleClear();
    this->m_packetsReceived.packetReceived();
    if (this->m_receiveMode != START) {
      this->m_file.osFile.close();
      this->m_warnings.invalidReceiveMode(Fw::FilePacket::T_START);
    }
    const Os::File::Status status = this->m_file.open(startPacket);
    if (status == Os::File::OP_OK) {
      this->m_packetCounter = 0;
      this->sendAck_out(0, packetID, source);
      this->m_packetCounter++;
      this->m_endPacketIndex = static_cast<U32>(this->m_file.size / FW_FILE_CHUNK_SIZE) + 1;
      std::cout << "[FileUplink] Expecting " << (unsigned)this->m_endPacketIndex << " packets" << std::endl;
      this->goToDataMode();
    }
    else {
      this->m_warnings.fileOpen(this->m_file.name);
      this->goToStartMode();
    }
  }

  void FileUplink::handleDataPacket(const Fw::FilePacket::DataPacket& dataPacket, U8 packetID, U8 source)
  {
    this->m_packetsReceived.packetReceived();
    if (this->m_receiveMode != DATA) {
      this->m_warnings.invalidReceiveMode(Fw::FilePacket::T_DATA);
      return;
    }
    const U32 sequenceIndex = dataPacket.asHeader().getSequenceIndex();
    this->checkSequenceIndex(sequenceIndex);
    const U32 byteOffset = dataPacket.getByteOffset();
    const U32 dataSize = dataPacket.getDataSize();
    if (byteOffset + dataSize > this->m_file.size) {
      this->m_warnings.packetOutOfBounds(sequenceIndex, this->m_file.name);
      return;
    }
    const Os::File::Status status = this->m_file.write(
      dataPacket.getData(),
      byteOffset,
      dataSize
    );
    if (status != Os::File::OP_OK) {
      this->m_warnings.fileWrite(this->m_file.name);
    }
    else {
      // TODO check if should add an else here 
      this->sendAck_out(0, packetID, source);
    }
    //this->m_packetCounter++;

  }

  void FileUplink::handleEndPacket(const Fw::FilePacket::EndPacket& endPacket, U8 packetID, U8 source)
  {
    this->m_packetsReceived.packetReceived();
    std::cout << "[FileUplink] Received end at packet nb " << (unsigned)this->m_packetCounter << std::endl;
    //if (this->m_receiveMode == DATA) {
    if (this->m_receiveMode == DATA && this->m_packetCounter >= this->m_endPacketIndex) {
      this->m_filesReceived.fileReceived();
      std::cout << "[FileUplink] Correctly received everything" << std::endl;
      this->checkSequenceIndex(endPacket.asHeader().getSequenceIndex());
      this->compareChecksums(endPacket);
      this->log_ACTIVITY_HI_FileReceived(this->m_file.name);
      // TODO here we need to send the ack only if all the packets were sent
      this->sendAck_out(0, packetID, source);
      this->m_packetCounter = 0;
      this->m_endPacketIndex = 0;
      this->goToStartMode();
    }
    else {
      this->m_warnings.invalidReceiveMode(Fw::FilePacket::T_END);
    }
    //TODO implement a time out
    //this->goToStartMode();
  }

  void FileUplink::handleCancelPacket()
  {
    this->m_packetsReceived.packetReceived();
    this->log_ACTIVITY_HI_UplinkCanceled();
    this->goToStartMode();
  }

  void FileUplink::checkSequenceIndex(const U32 sequenceIndex)
  {
    if (sequenceIndex != this->m_lastSequenceIndex + 1)
    {
      this->m_warnings.packetOutOfOrder(
        sequenceIndex,
        this->m_lastSequenceIndex
      );
    }
    else
    {
      this->m_packetCounter++;
    }
    this->m_lastSequenceIndex = sequenceIndex;
  }

  void FileUplink::compareChecksums(const Fw::FilePacket::EndPacket& endPacket)
  {
    CFDP::Checksum computed, stored;
    this->m_file.getChecksum(computed);
    endPacket.getChecksum(stored);
    if (computed != stored) {
      this->m_warnings.badChecksum(
        computed.getValue(),
        stored.getValue()
      );
    }
  }

  void FileUplink::goToStartMode()
  {
    this->m_file.osFile.close();
    this->m_receiveMode = START;
    this->m_lastSequenceIndex = 0;
  }

  void FileUplink::goToDataMode()
  {
    this->m_receiveMode = DATA;
    this->m_lastSequenceIndex = 0;
  }

}
