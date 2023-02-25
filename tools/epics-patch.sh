#GNU         = NO
#CMPLR_CLASS = clang
#CC          = clang
#CCC         = clang++

sed -i "s/\#COMMANDLINE_LIBRARY = EPICS/COMMANDLINE_LIBRARY = EPICS/" $1/epics/src/epics/configure/os/CONFIG_SITE.Common.linux-x86_64
sed -i "s/\#GNU         = NO/GNU         = NO/" $1/epics/src/epics/configure/os/CONFIG_SITE.linux-x86_64.linux-x86_64
sed -i "s/\#CMPLR_CLASS = clang/CMPLR_CLASS = clang/" $1/epics/src/epics/configure/os/CONFIG_SITE.linux-x86_64.linux-x86_64
sed -i "s/\#CC          = clang/CC          = clang/" $1/epics/src/epics/configure/os/CONFIG_SITE.linux-x86_64.linux-x86_64
sed -i "s/\#CCC         = clang++/CCC         = clang++/" $1/epics/src/epics/configure/os/CONFIG_SITE.linux-x86_64.linux-x86_64
sed -i "s/STATIC_BUILD=NO/STATIC_BUILD=YES/g" $1/epics/src/epics/configure/CONFIG_SITE
#sed -i "s/SHARED_LIBRARIES=YES/SHARED_LIBRARIES=NO/g" $1/epics/src/epics/configure/CONFIG_SITE