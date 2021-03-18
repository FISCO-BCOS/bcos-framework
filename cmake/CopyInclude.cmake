include(GNUInstallDirs)
set(DESTINATION_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/bcos-framework")
file(COPY interfaces 
DESTINATION ${DESTINATION_INCLUDE_DIR}
FILES_MATCHING PATTERN "*.h")

# remove the copied files
file(REMOVE ${DESTINATION_INCLUDE_DIR})

# utilities
file(COPY libutilities
    DESTINATION ${DESTINATION_INCLUDE_DIR}
    FILES_MATCHING PATTERN "*.h"
)

# codec
file(COPY libcodec
    DESTINATION ${DESTINATION_INCLUDE_DIR}
    FILES_MATCHING PATTERN "*.h"
)

# protocol
file(COPY libprotocol
    DESTINATION ${DESTINATION_INCLUDE_DIR}
    FILES_MATCHING PATTERN "*.h"
)