# -*- cmake -*-

include(00-Common)
include(LLCommon)
include(LLImage)
include(LLInventory)
include(LLMath)
include(LLMessage)
include(LLPrimitive)
include(LLRender)
include(LLWindow)
include(LLXML)
include(mimesh)
include(Linking)
include(UI)

find_package(Libgsf REQUIRED)
find_package(Libmagic REQUIRED)

include_directories(
    ${LLCOMMON_INCLUDE_DIRS}
    ${LIBS_PREBUILT_DIR}/include/jpeglib  # Coz JPEG.cmake seems to want to grab the system headers instead.  Though it works fine for llimage.
    ${LLIMAGE_INCLUDE_DIRS}
    ${LLINVENTORY_INCLUDE_DIRS}
    ${LLMATH_INCLUDE_DIRS}
    ${LLMESSAGE_INCLUDE_DIRS}
    ${LLPRIMITIVE_INCLUDE_DIRS}
    ${LLRENDER_INCLUDE_DIRS}
    ${LLWINDOW_INCLUDE_DIRS}
    ${LLXML_INCLUDE_DIRS}
    ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/libxml2
    ${LIBGSF_INCLUDE_DIRS}
    ${LIBMAGIC_INCLUDE_DIRS}
    ${G3D_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/newview
)


macro(add_mimesh_library name type path)

    project(${name})

    # Note, will have to rebuild from scratch when adding or deleting a file.
    if (${ARGC} EQUAL 3)
	file(GLOB ${name}_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.c ${path}/*.cpp)
	file(GLOB ${name}_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.h)
    else (${ARGC} EQUAL 3)
	file(GLOB ${name}_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/${ARGV3})
    endif (${ARGC} EQUAL 3)

    list(APPEND ${name}_SOURCE_FILES ${${name}_HEADER_FILES})
    add_library(${name} ${type} ${${name}_SOURCE_FILES})

    if (DARWIN)
	# don't embed a full path in the library's install name
	set_target_properties(
	    ${name}
	    PROPERTIES
	    BUILD_WITH_INSTALL_RPATH 1
	    INSTALL_NAME_DIR "@executable_path/../Resources"
	    )
    endif (DARWIN)

#    set_target_properties(
#	 ${name}
#	PROPERTIES
#	LINK_FLAGS "-export-dynamic -no-undefined"
#    )

    target_link_libraries(${name} ${${name}_link_LIBRARIES})

    install(TARGETS ${name}
	ARCHIVE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}	# This is just be a dummy.
	LIBRARY DESTINATION lib/libg3d
	)

endmacro(add_mimesh_library)


set(g3d_link_LIBRARIES
    ${LIBGSF_LIBRARIES}
    ${LIBMAGIC_LIBRARIES}
    )
add_mimesh_library(g3d STATIC libg3d-0.0.8/src)
add_mimesh_library(mimesh STATIC ".")

#   3ds		- Still using g3d_texture_load, which I removed.
#   heightfield	- too buggy to use.
#   ldraw	- needs a ldraw parts library installed.
#   leocad	- needs a leocad parts library installed.
#   test	- no point, it's for testing.  lol
#   vrml	- failed to load ./lib/libg3d/libg3d_plugin_vrml.so: ./lib/libg3d/libg3d_plugin_vrml.so: undefined symbol: yywrap
add_mimesh_library(g3d_plugin_3dm SHARED libg3d-0.0.8/plugins/import/imp_3dm)
add_mimesh_library(g3d_plugin_3dmf SHARED libg3d-0.0.8/plugins/import/imp_3dmf)
#add_mimesh_library(g3d_plugin_3ds SHARED libg3d-0.0.8/plugins/import/imp_3ds)
add_mimesh_library(g3d_plugin_ac3d SHARED libg3d-0.0.8/plugins/import/imp_ac3d)
add_mimesh_library(g3d_plugin_acf SHARED libg3d-0.0.8/plugins/import/imp_acf)
add_mimesh_library(g3d_plugin_ar SHARED libg3d-0.0.8/plugins/import/imp_ar)
add_mimesh_library(g3d_plugin_ase SHARED libg3d-0.0.8/plugins/import/imp_ase)
add_mimesh_library(g3d_plugin_blend SHARED libg3d-0.0.8/plugins/import/imp_blend)
add_mimesh_library(g3d_plugin_c4d SHARED libg3d-0.0.8/plugins/import/imp_c4d)
add_mimesh_library(g3d_plugin_cob SHARED libg3d-0.0.8/plugins/import/imp_cob)
add_mimesh_library(g3d_plugin_dae SHARED libg3d-0.0.8/plugins/import/imp_dae)
add_mimesh_library(g3d_plugin_dpm SHARED libg3d-0.0.8/plugins/import/imp_dpm)
add_mimesh_library(g3d_plugin_dxf SHARED libg3d-0.0.8/plugins/import/imp_dxf)
add_mimesh_library(g3d_plugin_flt SHARED libg3d-0.0.8/plugins/import/imp_flt)
add_mimesh_library(g3d_plugin_glb SHARED libg3d-0.0.8/plugins/import/imp_glb)
#add_mimesh_library(g3d_plugin_heightfield SHARED libg3d-0.0.8/plugins/import/imp_heightfield)
add_mimesh_library(g3d_plugin_iob SHARED libg3d-0.0.8/plugins/import/imp_iob)
add_mimesh_library(g3d_plugin_joe SHARED libg3d-0.0.8/plugins/import/imp_joe)
add_mimesh_library(g3d_plugin_kmz SHARED libg3d-0.0.8/plugins/import/imp_kmz)
add_mimesh_library(g3d_plugin_ldraw SHARED libg3d-0.0.8/plugins/import/imp_ldraw)
add_mimesh_library(g3d_plugin_leocad SHARED libg3d-0.0.8/plugins/import/imp_leocad)
add_mimesh_library(g3d_plugin_lwo SHARED libg3d-0.0.8/plugins/import/imp_lwo)
add_mimesh_library(g3d_plugin_max SHARED libg3d-0.0.8/plugins/import/imp_max)
add_mimesh_library(g3d_plugin_maya SHARED libg3d-0.0.8/plugins/import/imp_maya)
add_mimesh_library(g3d_plugin_md2 SHARED libg3d-0.0.8/plugins/import/imp_md2)
add_mimesh_library(g3d_plugin_md3 SHARED libg3d-0.0.8/plugins/import/imp_md3)
add_mimesh_library(g3d_plugin_nff SHARED libg3d-0.0.8/plugins/import/imp_nff)
add_mimesh_library(g3d_plugin_obj SHARED libg3d-0.0.8/plugins/import/imp_obj)
add_mimesh_library(g3d_plugin_osm SHARED libg3d-0.0.8/plugins/import/imp_osm)
add_mimesh_library(g3d_plugin_q3o SHARED libg3d-0.0.8/plugins/import/imp_q3o)
add_mimesh_library(g3d_plugin_r4 SHARED libg3d-0.0.8/plugins/import/imp_r4)
add_mimesh_library(g3d_plugin_rbh SHARED libg3d-0.0.8/plugins/import/imp_rbh)
add_mimesh_library(g3d_plugin_skp SHARED libg3d-0.0.8/plugins/import/imp_skp)
add_mimesh_library(g3d_plugin_stl SHARED libg3d-0.0.8/plugins/import/imp_stl)
add_mimesh_library(g3d_plugin_ta SHARED libg3d-0.0.8/plugins/import/imp_ta)
#add_mimesh_library(g3d_plugin_test SHARED libg3d-0.0.8/plugins/import/imp_test)
#add_mimesh_library(g3d_plugin_vrml SHARED libg3d-0.0.8/plugins/import/imp_vrml)
add_mimesh_library(g3d_plugin_vrml2 SHARED libg3d-0.0.8/plugins/import/imp_vrml2)


set(g3d_plugin_gdkpixbuf_link_LIBRARIES
    ${LIBGTK_LIBRARIES}
    ${LIBCAIRO_LIBRARIES}
    ${LIBPANGO_LIBRARIES}
    )
add_mimesh_library(g3d_plugin_gdkpixbuf SHARED libg3d-0.0.8/plugins/image img_gdkpixbuf.c)

set(g3d_plugin_jpeg_link_LIBRARIES
    ${JPEG_LIBRARIES}
    )
add_mimesh_library(g3d_plugin_jpeg SHARED libg3d-0.0.8/plugins/image img_jpeg.c)

add_mimesh_library(g3d_plugin_bmp SHARED libg3d-0.0.8/plugins/image img_bmp.c)
add_mimesh_library(g3d_plugin_dds SHARED libg3d-0.0.8/plugins/image img_dds.c)
add_mimesh_library(g3d_plugin_sgi SHARED libg3d-0.0.8/plugins/image img_sgi.c)

install(FILES libg3d-0.0.8/libg3d.magic DESTINATION lib/libg3d)


