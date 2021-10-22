
#-----------------------------------------------------------------------------
# Helper function for setting build options via config-site file.
# Usage: silo_option_default(VAR <var-to-be-set> VALUE <value-for-var> TYPE <type-for-var>)
#        TYPE is optional, and PATH is assumed if TYPE is not set.
#-----------------------------------------------------------------------------
function(silo_option_default)
    set(options) # not used
    set(oneValueArgs VAR VALUE TYPE)
    set(multiValueArgs) # not used
    cmake_parse_arguments(sod "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN} )

    # required VAR argument
    if(NOT DEFINED sod_VAR)
        message(WARNING "silo_option_default requires VAR argument.")
        return()
    elseif("VAR" IN_LIST sod_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_option_default: VAR argument is missing a value.")
        return()
    endif()
    # required VALUE argument
    if(NOT DEFINED sod_VALUE)
        message(WARNING "silo_option_default: requires VALUE argument")
        return()
    elseif("VALUE" IN_LIST sod_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_option_default: VALUE argument is missing a value.")
        return()
    endif()
    # optional TYPE argument, if not specified PATH is assumed
    if(NOT DEFINED sod_TYPE)
        set(sod_TYPE "PATH")
    elseif("TYPE" IN_LIST sod_KEYWORDS_MISSING_VALUES)
        message(WARNING "silo_option_default: TYPE argument is missing a value.")
        return()
    endif()

    set(${sod_VAR} ${sod_VALUE} CACHE ${sod_TYPE} "${sod_VAR} value" FORCE)
endfunction()

