function(add_rabbitvcs_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE rabbitvcs_core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_rabbitvcs_test(statusparser_test)
add_rabbitvcs_test(plugin_test)
