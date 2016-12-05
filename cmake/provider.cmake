if(PROVIDER.CMA)
    set(pMR_PROVIDER_CMA PROVIDER.CMA)
    include(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(process_vm_writev HAVE_CMA)
    if(NOT HAVE_CMA)
        ERROR("Cross Memory Attach not available!")
    endif()

    add_subdirectory(providers/cma)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_CMA>)
    set(PROVIDER_STRING "${PROVIDER_STRING} CMA")
endif()

if(PROVIDER.MPI)
    set(pMR_PROVIDER_MPI PROVIDER.MPI)
    if(NOT ${BACKEND} STREQUAL "MPI")
        ERROR("MPI provider only available for backend MPI!")
    endif()

    if(MPI.PERSISTENT)
        set(MPI_CAPS "${MPI_CAPS} Persistent")
        add_definitions(-DMPI_PERSISTENT)
    endif()

    add_subdirectory(providers/mpi)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_MPI>)
    set(PROVIDER_STRING "${PROVIDER_STRING} MPI")
    INFO("MPI: Capabilities:${MPI_CAPS}")
endif()

if(PROVIDER.NULL)
    set(pMR_PROVIDER_NULL PROVIDER.NULL)
    if(NULL.WARN)
        add_definitions(-DNULL_WARN)
    endif()
    add_subdirectory(providers/null)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_NULL>)
    set(PROVIDER_STRING "${PROVIDER_STRING} null")
endif()

if(PROVIDER.NUMA)
    find_library(NUMA NAMES numa PATHS ENV LIBRARY_PATH)
    if(NOT NUMA)
        ERROR("Library NUMA not found!")
    endif()
    add_subdirectory(providers/numa)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_NUMA>)
    list(APPEND LIBRARIES ${NUMA})
    set(AUX_PROVIDER_STRING "${AUX_PROVIDER_STRING} NUMA")
endif()

if(PROVIDER.OFI)
    set(pMR_PROVIDER_OFI PROVIDER.OFI)
    find_library(FABRIC NAMES fabric PATHS ENV LIBRARY_PATH)
    if(NOT FABRIC)
        ERROR("Library fabric not found!")
    endif()

    if(NOT OFI.EP STREQUAL "MSG" AND NOT OFI.EP STREQUAL "RDM")
        ERROR("OFI: Unsupported endpoint type: ${OFI.EP}")
    endif()
    add_definitions(-DOFI_EP_${OFI.EP})

    if(OFI.RMA)
        set(OFI_CAPS "${OFI_CAPS} RMA")
        add_definitions(-DOFI_RMA)
        if(OFI.RMA.EVENT)
            set(OFI_CAPS "${OFI_CAPS} RMAEvent")
            add_definitions(-DOFI_RMA_EVENT)
        elseif(OFI.RMA.TARGET.RX)
            set(OFI_CAPS "${OFI_CAPS} RMATargetRx")
            add_definitions(-DOFI_RMA_TARGET_RX)
        endif()
    endif()

    if(OFI.CONTROL.PROGRESS.AUTO)
        set(OFI_CAPS "${OFI_CAPS} ControlProgressAuto")
        add_definitions(-DOFI_CONTROL_PROGRESS_AUTO)
    endif()

    if(OFI.DATA.PROGRESS.AUTO)
        set(OFI_CAPS "${OFI_CAPS} DataProgressAuto")
        add_definitions(-DOFI_DATA_PROGRESS_AUTO)
    endif()

    if(OFI.RM)
        set(OFI_CAPS "${OFI_CAPS} RM")
        add_definitions(-DOFI_RM)
    endif()

    if(OFI.MR.SCALABLE)
        set(OFI_CAPS "${OFI_CAPS} MRScalable")
        add_definitions(-DOFI_MR_SCALABLE)
    endif()

    add_subdirectory(providers/ofi)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_OFI>)
    list(APPEND LIBRARIES ${FABRIC})
    set(PROVIDER_STRING "${PROVIDER_STRING} OFI")
    INFO("OFI: Endpoint ${OFI.EP} with capabilities:${OFI_CAPS}")
endif()

if(PROVIDER.SCIF)
    find_library(SCIF NAMES scif PATHS ENV LIBRARY_PATH)
    if(NOT SCIF)
        ERROR("Library scif not found!")
    endif()
    add_subdirectory(providers/scif)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_SCIF>)
    list(APPEND LIBRARIES ${SCIF})
    set(AUX_PROVIDER_STRING "${AUX_PROVIDER_STRING} SCIF")
endif()

if(PROVIDER.SELF)
    set(pMR_PROVIDER_SELF PROVIDER.SELF)
    if(SELF.WARN)
        add_definitions(-DSELF_WARN)
    endif()
    add_subdirectory(providers/self)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_SELF>)
    set(PROVIDER_STRING "${PROVIDER_STRING} self")
endif()

if(PROVIDER.VERBS)
    set(pMR_PROVIDER_VERBS PROVIDER.VERBS)
    find_library(IBVERBS NAMES ibverbs PATHS ENV LIBRARY_PATH)
    if(NOT IBVERBS)
        ERROR("Library ibverbs not found!")
    endif()

    if(VERBS.RDMA)
        set(VERBS_CAPS "${VERBS_CAPS} RDMA")
        add_definitions(-DVERBS_RDMA)
    endif()

    if(VERBS.MTU STREQUAL 256)
        add_definitions(-DVERBS_MTU=IBV_MTU_256)
    elseif(VERBS.MTU STREQUAL 512)
        add_definitions(-DVERBS_MTU=IBV_MTU_512)
    elseif(VERBS.MTU STREQUAL 1024)
        add_definitions(-DVERBS_MTU=IBV_MTU_1024)
    elseif(VERBS.MTU STREQUAL 2048)
        add_definitions(-DVERBS_MTU=IBV_MTU_2048)
    elseif(VERBS.MTU STREQUAL 4096)
        add_definitions(-DVERBS_MTU=IBV_MTU_4096)
    else()
        ERROR("verbs: Unsupported MTU size.")
    endif()
    set(VERBS_CAPS "${VERBS_CAPS} MTU=${VERBS.MTU}")

    if(VERBS.INLINE STREQUAL "")
        ERROR("verbs: No max inline data size set")
    endif()
    add_definitions(-DVERBS_INLINE=${VERBS.INLINE})
    set(VERBS_CAPS "${VERBS_CAPS} MaxInlineData=${VERBS.INLINE}")

    if(VERBS.ODP)
        include(CheckStructHasMember)
        check_struct_has_member("struct ibv_device_attr_ex" odp_caps infiniband/verbs.h HAVE_IBVERBS_ODP LANGUAGE C)
        if(HAVE_IBVERBS_ODP)
            set(VERBS_CAPS "${VERBS_CAPS} ODP")
            add_definitions(-DVERBS_ODP)
        endif()
    endif()

    add_subdirectory(providers/verbs)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_VERBS>)
    list(APPEND LIBRARIES ${IBVERBS})
    set(PROVIDER_STRING "${PROVIDER_STRING} verbs")
    INFO("verbs: Capabilities:${VERBS_CAPS}")
endif()

INFO("Enabled provider(s):${PROVIDER_STRING}")

if(AUX_PROVIDER_STRING)
    INFO("Enabled auxiliary provider(s):${AUX_PROVIDER_STRING}")
endif()
