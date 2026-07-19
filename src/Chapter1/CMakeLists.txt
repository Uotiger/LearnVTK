add_executable(Chapter1_DataModelDemo
    main.cpp
    DataModelDemo.h
    DataModelDemo.cpp
)

target_link_libraries(Chapter1_DataModelDemo PRIVATE
    ${VTK_LIBRARIES}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

set_target_properties(Chapter1_DataModelDemo PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

deploy_qt_for_target(Chapter1_DataModelDemo)
