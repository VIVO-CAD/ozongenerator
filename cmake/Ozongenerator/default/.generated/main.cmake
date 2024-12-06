include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")
set(Ozongenerator_default_library_list )
# Handle files with suffix (s|as|asm|AS|ASM|As|aS|Asm) 
if(Ozongenerator_default_FILE_GROUP_assemble)
    add_library(Ozongenerator_default_assemble OBJECT ${Ozongenerator_default_FILE_GROUP_assemble})
    Ozongenerator_default_assemble_rule(Ozongenerator_default_assemble)
    list(APPEND Ozongenerator_default_library_list "$<TARGET_OBJECTS:Ozongenerator_default_assemble>")
endif()

# Handle files with suffix S 
if(Ozongenerator_default_FILE_GROUP_assemblePreprocess)
    add_library(Ozongenerator_default_assemblePreprocess OBJECT ${Ozongenerator_default_FILE_GROUP_assemblePreprocess})
    Ozongenerator_default_assemblePreprocess_rule(Ozongenerator_default_assemblePreprocess)
    list(APPEND Ozongenerator_default_library_list "$<TARGET_OBJECTS:Ozongenerator_default_assemblePreprocess>")
endif()

# Handle files with suffix [cC] 
if(Ozongenerator_default_FILE_GROUP_compile)
    add_library(Ozongenerator_default_compile OBJECT ${Ozongenerator_default_FILE_GROUP_compile})
    Ozongenerator_default_compile_rule(Ozongenerator_default_compile)
    list(APPEND Ozongenerator_default_library_list "$<TARGET_OBJECTS:Ozongenerator_default_compile>")
endif()

if (BUILD_LIBRARY)
        message(STATUS "Building LIBRARY")
        add_library(${Ozongenerator_default_image_name} ${Ozongenerator_default_library_list})
        foreach(lib ${Ozongenerator_default_FILE_GROUP_link})
        target_link_libraries(${Ozongenerator_default_image_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} /${lib})
        endforeach()
        add_custom_command(
            TARGET ${Ozongenerator_default_image_name}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${Ozongenerator_default_output_dir}
    COMMAND ${CMAKE_COMMAND} -E copy lib${Ozongenerator_default_image_name}.a ${Ozongenerator_default_output_dir}/${Ozongenerator_default_original_image_name})
else()
    message(STATUS "Building STANDARD")
    add_executable(${Ozongenerator_default_image_name} ${Ozongenerator_default_library_list})
    foreach(lib ${Ozongenerator_default_FILE_GROUP_link})
    target_link_libraries(${Ozongenerator_default_image_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/${lib})
endforeach()
    Ozongenerator_default_link_rule(${Ozongenerator_default_image_name})
    
add_custom_command(
    TARGET ${Ozongenerator_default_image_name}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${Ozongenerator_default_output_dir}
    COMMAND ${CMAKE_COMMAND} -E copy ${Ozongenerator_default_image_name} ${Ozongenerator_default_output_dir}/${Ozongenerator_default_original_image_name})
endif()
