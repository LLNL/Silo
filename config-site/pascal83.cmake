###
# config-site file for pascal
###

###
# include general settings
###
# should the linux.cmake be x86_64.cmake instead?
include(config-site/Linux.cmake)
include(config-site/llnl_lc.cmake)

###
#  Options specific to pascal
###
# anything other options that should be enabled/disabled for pascal?
#  SILO_PARALLEL?  (not yet enabled in root CMakeLists.txt though
#  SILO_ENABLE_TESTS?

# since llnl_lc.cmake defines location of Qt5, should that file be turning this on?
silo_option_default(VAR SILO_BUILD_SILEX VALUE ON TYPE BOOL)
