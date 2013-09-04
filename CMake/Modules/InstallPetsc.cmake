#InstallPetsc.cmake
#
# Created on: Jul 20, 2012
#     Author: jb
#
# accepted variables: 
#     EXTERNAL_PETSC_DIR - target directory used for instalation
#     INSTALL_PETSC_URL - url with petsc tarball
#     PETSC_MPI_DIR      - pass MPI to PETSC configure 
#     PETSC_LAPACK_DIR   - pass Lapack to PETSC configure
#     INSTALL_PETSC_ONLY - install only petsc (possibly MPI and BLAS/LAPACK)
#     INSTALL_PETSC_OPTIONS - add content of this variable to the PETSC configure command
#     (default)          - install also, metis, parmetis



if (NOT EXTERNAL_PETSC_DIR)
    set(EXTERNAL_PETSC_DIR "${EXTERNAL_PROJECT_DIR}/petsc_build")
endif()    

##########################################################################
# Download PETSC
# A temporary CMakeLists.txt

# set configure line for PETSC
set(PETSC_CONF_LINE "--COPTFLAGS ${CMAKE_C_FLAGS} --CXXOPTFLAGS ${CMAKE_CXX_FLAGS} --with-clanguage=C --with-debugging=0")
if (CMAKE_Fortran_COMPILER)
    set(PETSC_CONF_LINE "${PETSC_CONF_LINE} --FOPTFLAGS ${CMAKE_Fortran_FLAGS}")
endif()

if (PETSC_MPI_DIR)
    set(PETSC_CONF_LINE "${PETSC_CONF_LINE} --with-mpi-dir=${PETSC_MPI_DIR}")
else()
    set(PETSC_CONF_LINE "${PETSC_CONF_LINE} --download-mpich=yes")
endif()

if (PETSC_LAPACK_DIR)
    set(PETSC_CONF_LINE "${PETSC_CONF_LINE} --with-lapack-dir=${PETSC_LAPACK_DIR}")
else()
    if(CMAKE_Fortran_COMPILER)
        set(PETSC_CONF_LINE "${PETSC_CONF_LINE} --download-f-blas-lapack=yes")
    else()
        set(PETSC_CONF_LINE "${PETSC_CONF_LINE} --download-c-blas-lapack=yes")
    endif()    
endif()

if(INSTALL_PETSC_ONLY)
else()
    set(PETSC_CONF_LINE "${PETSC_CONF_LINE} --download-metis=yes --download-parmetis=yes")
endif()    

set(PETSC_CONF_LINE "${PETSC_CONF_LINE} ${INSTALL_PETSC_OPTIONS}")

set (cmakelists_fname "${EXTERNAL_PETSC_DIR}/CMakeLists.txt")
file (WRITE "${cmakelists_fname}"
"
  ## This file was autogenerated by InstallPETSC.cmake
  cmake_minimum_required(VERSION 2.8)
  include(ExternalProject)
  ExternalProject_Add(PETSC
    DOWNLOAD_DIR ${EXTERNAL_PETSC_DIR} 
    URL ${INSTALL_PETSC_URL}
    SOURCE_DIR ${EXTERNAL_PETSC_DIR}/src
    BINARY_DIR ${EXTERNAL_PETSC_DIR}/src
    CONFIGURE_COMMAND ${EXTERNAL_PETSC_DIR}/src/configure ${PETSC_CONF_LINE}
    BUILD_COMMAND make all
    INSTALL_COMMAND \"\"
  )  
")

message(STATUS "=== Installing PETSC ===")
# run cmake
set(PETSC_DIR "${EXTERNAL_PETSC_DIR}/src")
set(PETSC_ARCH "cmake")
set(ENV{PETSC_DIR} "${PETSC_DIR}")
set(ENV{PETSC_ARCH} "${PETSC_ARCH}")
execute_process(COMMAND ${CMAKE_COMMAND} ${EXTERNAL_PETSC_DIR} 
      WORKING_DIRECTORY ${EXTERNAL_PETSC_DIR})

find_program (MAKE_EXECUTABLE NAMES make gmake)
# run make
execute_process(COMMAND ${MAKE_EXECUTABLE} PETSC
      WORKING_DIRECTORY ${EXTERNAL_PETSC_DIR})    


#file (REMOVE ${cmakelists_fname})

message(STATUS "== PETSC build done")

