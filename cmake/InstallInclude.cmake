install(
    DIRECTORY "libutilities/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/bcos-framework/libutilities"
    FILES_MATCHING PATTERN "*.h"
)

install(
    DIRECTORY "interfaces"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/bcos-framework"
    FILES_MATCHING PATTERN "*.h"
)
