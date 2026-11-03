# Code shared by both plugins and the tests.
add_library(rabbitvcs_core STATIC rabbitvcsplugin.cpp statusparser.cpp)
set_target_properties(rabbitvcs_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(rabbitvcs_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${DOLPHINVCS_INCLUDE_DIR})
target_link_libraries(rabbitvcs_core PUBLIC Qt6::Core Qt6::Widgets KF6::KIOCore KF6::KIOGui DolphinVcs)

function(add_rabbitvcs_plugin target source)
    kcoreaddons_add_plugin(${target} INSTALL_NAMESPACE "dolphin/vcs")
    target_sources(${target} PRIVATE ${source})
    target_link_libraries(${target} PRIVATE rabbitvcs_core)
endfunction()

add_rabbitvcs_plugin(rabbitvcsgitplugin gitplugin.cpp)
add_rabbitvcs_plugin(rabbitvcssvnplugin svnplugin.cpp)
