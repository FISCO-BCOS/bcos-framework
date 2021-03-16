# utilities
install(
    DIRECTORY "libutilities/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/bcos-framework/libutilities"
    FILES_MATCHING PATTERN "*.h"
)

# interfaces
install(
    DIRECTORY "interfaces"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/bcos-framework"
    FILES_MATCHING PATTERN "*.h"
)

# codec
install(
    DIRECTORY "libcodec"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/bcos-framework/libcodec"
    FILES_MATCHING PATTERN "*.h"
)