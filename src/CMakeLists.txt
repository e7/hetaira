aux_source_directory(. src_files)
aux_source_directory(business busi_src_files)
aux_source_directory(business/mobile mobile_src_files)

add_executable(gw-mobile ${src_files} ${busi_src_files} ${mobile_src_files})
target_link_libraries(gw-mobile mysqlcppconn-static.a pthread.so dl.so crypto.a)
