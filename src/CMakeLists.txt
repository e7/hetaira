aux_source_directory(. src_files)
aux_source_directory(business busi_src_files)

add_executable(hetaira ${src_files} ${busi_src_files} ${mobile_src_files})
target_link_libraries(hetaira pthread.so dl.so crypto.a)
