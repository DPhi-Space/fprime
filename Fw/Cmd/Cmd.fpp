module Fw {

  type CmdArgBuffer

  @ Command registration port
  port CmdReg(
               opCode: FwOpcodeType @< Command Op Code
             )

  @ Port for sending commands
  port Cmd(
            opCode: FwOpcodeType @< Command Op Code
            cmdSeq: U32 @< Command Sequence
            ref args: CmdArgBuffer @< Buffer containing arguments
          )

  @ Enum representing a command response
  enum CmdResponse : U8 {
    OK = 0 @< Command successfully executed
    INVALID_OPCODE = 1 @< Invalid opcode dispatched
    VALIDATION_ERROR = 2 @< Command failed validation
    FORMAT_ERROR = 3 @< Command failed to deserialize
    EXECUTION_ERROR = 4 @< Command had execution error
    BUSY = 5 @< Component busy

    # DOCKER ERRORS
    DOCKER_TARFILE_NOT_FOUND = 10,
    DOCKER_TARFILE_LOAD_FAILED = 11 ,
    DOCKER_BUILD_FAILED = 12,
    DOCKER_DOCKERFILE_NOT_FOUND = 13 ,
    DOCKER_EXEC_FORMAT_ERROR = 14,
    DOCKER_IMAGE_NOT_FOUND = 15,
    DOCKER_IMAGE_LOAD_FAILED = 16,
    DOCKER_ERROR_PULLING_IMAGE = 17,

    # K3S ERRORS
  }

  @ Port for sending command responses
  port CmdResponse(
                    opCode: FwOpcodeType @< Command Op Code
                    cmdSeq: U32 @< Command Sequence
                    response: CmdResponse @< The command response argument
                  )

  @ Port for sending command responses with data
  port CmdResponseData(
                    opCode: FwOpcodeType @< Command Op Code
                    cmdSeq: U32 @< Command Sequence
                    response: CmdResponse @< The command response argument
                    data: ComBuffer
                  )

}
