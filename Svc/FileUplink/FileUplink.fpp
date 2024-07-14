module Svc {

  @ A component for uplinking files
  active component FileUplink {

    # ----------------------------------------------------------------------
    # General Ports
    # ----------------------------------------------------------------------

    @ Buffer send in
    async input port bufferSendIn: Fw.BufferSend

    @ Buffer send out
    output port bufferSendOut: Fw.BufferSend

    @ Ping in
    async input port pingIn: Svc.Ping

    @ Ping out
    output port pingOut: Svc.Ping

    @ Port for sending an ack back for the FilePkt when it is valid
    output port sendAck: Components.PktAck

    # ------------------------------
    # Special Ports
    # ----------------------------------------------------------------------

    @ Time get
    time get port timeCaller

    @ Telemetry
    telemetry port tlmOut

    @ Event
    event port eventOut

    @ Text event
    text event port LogText

    # ----------------------------------------------------------------------
    # Telemetry
    # ----------------------------------------------------------------------

    include "Telemetry.fppi"

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------

    include "Events.fppi"

  }

}
