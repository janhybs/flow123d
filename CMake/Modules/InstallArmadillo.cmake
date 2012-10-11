#InstallArmadillo.cmake
#
# Created on: Jul 20, 2012
#     Author: jb
#

# set directory to which install armadillo 
if (NOT EXTERNAL_ARMADILLO_DIR)
    set(EXTERNAL_ARMADILLO_DIR "${PROJECT_BINARY_DIR}/armadillo_build")
endif()    
        
  # check PETSC fblas and flapack
  find_file(PETSC_BLAS libfblas.a PATH  ${PETSC_DIR}/${PETSC_ARCH}/lib)
  find_file(PETSC_LAPACK libflapack.a PATH  ${PETSC_DIR}/${PETSC_ARCH}/lib)
  if ( PETSC_BLAS AND PETSC_LAPACK )
      set(PETSC_LAPACK "-DLAPACK_LIBRARY=${PETSC_LAPACK};libmpi.a;libgfortran.a")
      set(PETSC_BLAS "-DBLAS_LIBRARY=${PETSC_BLAS};libmpi.a;libgfortran.a")
  else()
      # if fblas or flapack is not presented in PETSC let armadillo to find its own BLAS and LAPACK
      set(PETSC_LAPACK "")
      set(PETSC_BLAS "")
  endif()    
  
  # hint same Boost to armadillo 
  set(Armadillo_Boost_Hint  -D BOOST_ROOT=${BOOST_ROOT})



# A temporary CMakeLists.txt
set (cmakelists_fname "${EXTERNAL_ARMADILLO_DIR}/CMakeLists.txt")
message(STATUS ${cmakelists_fname})
file (WRITE "${cmakelists_fname}"
"
  ## This file was autogenerated by InstallArmadillo.cmake
  cmake_minimum_required(VERSION 2.8)
  include(ExternalProject)
  ExternalProject_Add(Armadillo
    DOWNLOAD_DIR ${EXTERNAL_ARMADILLO_DIR} 
    URL \"http://dev.nti.tul.cz/~brezina/flow123d_libraries/armadillo-3.4.3.tar.gz\"
    SOURCE_DIR ${EXTERNAL_ARMADILLO_DIR}/src
    BINARY_DIR ${EXTERNAL_ARMADILLO_DIR}/src
    PATCH_COMMAND patch ${EXTERNAL_ARMADILLO_DIR}/src/CMakeLists.txt ${PROJECT_SOURCE_DIR}/third_party/armadillo_patch
    CONFIGURE_COMMAND cmake ${Armadillo_Boost_Hint} ${PETSC_BLAS} ${PETSC_LAPACK} -DCMAKE_INSTALL_PREFIX=${EXTERNAL_ARMADILLO_DIR} .
    BUILD_COMMAND make
    INSTALL_COMMAND make install 
  )
")

# run cmake
execute_process(COMMAND ${CMAKE_COMMAND} ${EXTERNAL_ARMADILLO_DIR} 
    WORKING_DIRECTORY ${EXTERNAL_ARMADILLO_DIR})

find_program (MAKE_EXECUTABLE NAMES make gmake)
# run make
execute_process(COMMAND ${MAKE_EXECUTABLE} 
    WORKING_DIRECTORY ${EXTERNAL_ARMADILLO_DIR})    


#file (REMOVE ${cmakelists_fname})

message(STATUS "Armadillo build done")