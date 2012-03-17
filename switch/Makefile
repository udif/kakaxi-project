

CC:=arm-linux-gcc
#CC:=gcc

ifeq "$(CC)" "arm-linux-gcc"
CFLAGS += -fno-strict-aliasing
CFLAGS += -DARCH_ARM
endif

ifeq "$(CC)" "gcc"
CROSS_COMPILE:=
CFLAGS += -Wno-unused-but-set-variable
endif

target_dir:=$(M_APP_PATH)/../libs/imx28/build
BLTEST_PATH:=$(target_dir)/usr/local/include/bltest
CFLAGS += -I$(BLTEST_PATH)
CFLAGS += -I$(target_dir)/usr/include
CFLAGS += -I$(target_dir)/usr/local/include
LDFLAGS += -L$(M_APP_PATH)/../libs/imx28/pkgs/usr/lib
LDFLAGS += -L$(M_APP_PATH)/../libs/imx28/pkgs/usr/local/lib

#gcov_cflags:=-fprofile-arcs -ftest-coverage

CFLAGS += -g -pipe -Wall -Werror $(gcov_cflags)
CFLAGS += -O2

CFLAGS+=-I../../include
CFLAGS+=-I../../include/common/sys

LDFLAGS+=
#-lcheck -lpthread


#------------------------------------------------------------------

target:=
# target+=cmc_oamd.out
# cmc_oamd.out-objs:=
# cmc_oamd.out-objs+=cmc_oamd/cmc_oam_main.o
# cmc_oamd.out-objs+=cmc_oamd/cmc_oam_task.o
# cmc_oamd.out-objs+=cmc_oamd/cmc_oam_cmd.o
# cmc_oamd.out-objs+=../../common/sys/vns_sys.o
# cmc_oamd.out-objs+=../../common/sys/vns_fdevent.o

# cmc_oamd.out-objs+=oam_src/base/baseContainer.o
# cmc_oamd.out-objs+=oam_src/base/baseDbg.o
# cmc_oamd.out-objs+=oam_src/base/baseKni_linuxuser.o
# cmc_oamd.out-objs+=oam_src/base/baseNetworking_linuxuser.o

# cmc_oamd.out-objs+=oam_src/OamApi/cmcOamChannel.o
# cmc_oamd.out-objs+=oam_src/OamApi/cmcOamCm.o
# cmc_oamd.out-objs+=oam_src/OamApi/cmcOamLoadBalancing.o
# cmc_oamd.out-objs+=oam_src/OamApi/cmcOamMsgQ.o
# cmc_oamd.out-objs+=oam_src/OamApi/cmcOamQos.o
# cmc_oamd.out-objs+=oam_src/OamApi/cmcOamXpt.o
# cmc_oamd.out-objs+=oam_src/OamApi/cmcOamSystem.o
# cmc_oamd.out-objs+=oam_src/OamApi/tlv.o
# cmc_oamd.out-objs+=oam_src/OamApi/tlvParseAPI.o

# cmc_oamd.out-cflags :=
# #cmc_oamd.out-cflags += -DNETWORKINGDEV_L2OAM
# cmc_oamd.out-cflags += -DNETWORKINGDEV_TCP
# cmc_oamd.out-cflags += -Ioam_src/base/include -Ioam_src/OamApi/include
# cmc_oamd.out-ldflags:= -lpthread


# target+=cmc_ctl.out
# cmc_ctl.out-objs:=cmc_ctl/ctl_cmd.o

#------------------------------------------------------------------

target+=blcmc.out
blcmc.out-objs:=
blcmc.out-objs+=blcmc/blcmc_main.o
blcmc.out-objs+=blcmc/blcmc_dmm.o
blcmc.out-objs+=blcmc/blcmc_trans.o
blcmc.out-objs+=blcmc/blcmc_utils.o

blcmc.out-objs+=blcmc/SetDownstreamConfig.o
blcmc.out-objs+=blcmc/GetDownstreamConfig.o
blcmc.out-objs+=blcmc/SetUpstreamConfig.o
blcmc.out-objs+=blcmc/GetUpstreamConfig.o
blcmc.out-objs+=blcmc/SetMoveCnuDownstream.o
blcmc.out-objs+=blcmc/GetMoveCnuDownstream.o
blcmc.out-objs+=blcmc/SetMoveCnuUpstream.o
blcmc.out-objs+=blcmc/GetMoveCnuUpstream.o
#blcmc.out-objs+=blcmc/SetMaxRfAttenuation.o
#blcmc.out-objs+=blcmc/GetMaxRfAttenuation.o
blcmc.out-objs+=blcmc/SetUpstreamInputPower.o
blcmc.out-objs+=blcmc/GetUpstreamInputPower.o

blcmc.out-objs+=blcmc/SetCreateLoadBalancingGroup.o
blcmc.out-objs+=blcmc/GetLoadBalancingGroups.o
blcmc.out-objs+=blcmc/SetAddDownstreamsToLdBalGrp.o
blcmc.out-objs+=blcmc/GetLdBalGrpDownstreams.o
blcmc.out-objs+=blcmc/SetAddUpstreamsToLdBalGrp.o
blcmc.out-objs+=blcmc/GetLdBalGrpUpstreams.o
blcmc.out-objs+=blcmc/SetAddCNUsToLdBalGrp.o
blcmc.out-objs+=blcmc/GetLdBalGrpCNUConfig.o
blcmc.out-objs+=blcmc/GetLdBalGrpActiveCNUs.o
blcmc.out-objs+=blcmc/SetExcludeCNUsFromLdBal.o
blcmc.out-objs+=blcmc/GetExcludedCNUsConfig.o
blcmc.out-objs+=blcmc/GetExcludedActiveCNUs.o
blcmc.out-objs+=blcmc/SetFullLoadBalancingGroup.o
blcmc.out-objs+=blcmc/GetFullLoadBalancingGroup.o
blcmc.out-objs+=blcmc/GetCnuStatus.o
blcmc.out-objs+=blcmc/GetCmcInterfacesData.o
blcmc.out-objs+=blcmc/GetCmcMacStatistics.o
blcmc.out-objs+=blcmc/GetServices.o
blcmc.out-objs+=blcmc/GetSignalQuality.o
blcmc.out-objs+=blcmc/GetCmcInterfaceStatistics.o
blcmc.out-objs+=blcmc/GetDownstreamBondingGroupStatus.o
blcmc.out-objs+=blcmc/GetUpstreamBondingGroupStatus.o
blcmc.out-objs+=blcmc/GetQosServiceFlowConfig.o
blcmc.out-objs+=blcmc/GetQosServiceFlowStatistics.o
blcmc.out-objs+=blcmc/GetQosPacketClassifierConfig.o
blcmc.out-objs+=blcmc/GetCmcSoftwareVersion.o
blcmc.out-objs+=blcmc/SetResetCmcBoard.o
blcmc.out-objs+=blcmc/GetCustomConfiguration.o
blcmc.out-objs+=blcmc/SetCustomConfiguration.o
blcmc.out-objs+=blcmc/SetFirmwareUpgrade.o
blcmc.out-objs+=blcmc/SetDsCalibrationData.o
blcmc.out-objs+=blcmc/GetDsCalibrationData.o
blcmc.out-objs+=blcmc/GetCnuList.o
blcmc.out-objs+=blcmc/SetAsyncLogConfig.o
blcmc.out-objs+=blcmc/GetAsyncLogConfig.o
blcmc.out-objs+=blcmc/GetLogData.o
blcmc.out-objs+=blcmc/SetUsCalibrationData.o
blcmc.out-objs+=blcmc/GetUsCalibrationData.o
blcmc.out-objs+=blcmc/SetUpstreamPhyRegister.o
blcmc.out-objs+=blcmc/GetUpstreamPhyRegister.o
blcmc.out-objs+=blcmc/GetAlarmList.o
blcmc.out-objs+=blcmc/SetRemoteCmd.o
blcmc.out-objs+=blcmc/GetCrashData.o
blcmc.out-objs+=blcmc/ClearCrashData.o
blcmc.out-objs+=blcmc/SetDownstreamTestMode.o
blcmc.out-objs+=blcmc/GetDownstreamTestMode.o
blcmc.out-objs+=

blcmc.out-cflags += -Ioam_src/base/include -Ioam_src/OamApi/include
#blcmc.out-cflags += -DBLCMC


target+=cmc_pm.out
cmc_pm.out-objs:=
cmc_pm.out-objs+=cmc_pm/main.o
cmc_pm.out-cflags += -Icmc_pm


target+=rtl8368.out
rtl8368.out-objs:=
rtl8368.out-objs+=switch/rtl8368.o
#rtl8368.out-objs+=switch/mdio/mdio.o
rtl8368.out-objs+=switch/rtl8368/rtk_api.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_acl.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_cputag.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_dot1x.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_eav.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_eee.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_fc.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_green.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_hsb.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_igmp.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_inbwctrl.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_interrupt.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_led.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_lut.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_meter.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_mib.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_mirror.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_misc.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_phy.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_port.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_portIsolation.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_qos.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_rma.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_scheduling.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_storm.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_svlan.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_trunking.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_unknownMulticast.o
rtl8368.out-objs+=switch/rtl8368/rtl8367b_asicdrv_vlan.o
rtl8368.out-objs+=switch/rtl8368/smi.o
rtl8368.out-cflags += -Iswitch/mdio
rtl8368.out-cflags += -Iswitch/rtl8368
rtl8368.out-cflags += -DMDC_MDIO_OPERATION
rtl8368.out-cflags += -D_LITTLE_ENDIAN
rtl8368.out-cflags += -DCHIP_RTL8368MB
rtl8368.out-cflags += -DDISABLE_VLAN_SHADOW


#------------------------------------------------------------------

all:clean mk_test

target-objs=$(foreach subtarget, $(target) ,$(subtarget)-objs )
all-objs=$(foreach obj, $(target-objs) ,$($(obj)) )
target-outs=$(foreach subtarget, $(target) ,$(subtarget))
#target-cflags=$(foreach subtarget, $(target) ,$(subtarget)-cflags )
#target-ldflags=$(foreach subtarget, $(target) ,$(subtarget)-ldflags )


$(target-outs): %: force
	$(CC) $(patsubst %.o,%.c,$($*-objs)) $(CFLAGS) $($*-cflags) $(LDFLAGS) $($*-ldflags) -o $@
	$(CROSS_COMPILE)strip $@

#------------------------------------------------------------------

#mib target
blmib-outs:=

# DOCS-IF-MIB
blmib-outs+=docsIfCmtsCmStatusTable.so
blmib-outs+=docsIfDownstreamChannelTable.so
blmib-outs+=docsIfUpstreamChannelTable.so

# DOCS-QOS3-MIB
#blmib-outs+=docsQosParamSetTable.so
#blmib-outs+=docsQosServiceFlowTable.so
#blmib-outs+=docsQosServiceFlowStatsTable.so
#blmib-outs+=docsQosCmtsMacToSrvFlowTable.so

# MIBs plans to add
# DOCS-IF-MIB
#blmib-outs+=docsIfCmtsServiceTable.so
blmib-outs+=docsIfCmtsMacToCmTable.so
blmib-outs+=docsIfSignalQualityTable.so
blmib-outs+=docsIfCmtsStatusTable.so
#blmib-outs+=docsIfCmtsModulationTable.so

# DOCS-IF3-MIB
#blmib-outs+=docsIf3CmtsCmRegStatusTable.so
#blmib-outs+=docsIf3BondingGrpCfgTable.so
#blmib-outs+=docsIf3DsBondingGrpStatusTable.so
#blmib-outs+=docsIf3UsBondingGrpStatusTable.so
#blmib-outs+=docsIf3SignalQualityExtTable.so
#blmib-outs+=docsIf3CmtsSignalQualityExtTable.so

# DOCS-QOS3-MIB
#blmib-outs+=docsQosPktClassTable.so
blmib-outs+=


blmib-objs=$(foreach su, $(blmib-outs) ,./cmc_mib/$(patsubst %.so,%.c,$(su)).c)
blmib-cflags=-fPIC --shared -I./cmc_mib -Wall -Werror -O2
blmib-cflags+=-I../../include/common/sys
blmib-cflags+=../../common/sys/vns_config.c
ifeq "$(M_CHIP_TYPE)" "IMX28"
blmib-cflags+=-fno-strict-aliasing
blmib-cflags+=-I$(target_dir)/usr/local/include
endif

$(blmib-outs): %: force
	$(CC) ./cmc_mib/$(patsubst %.so,%.c,$*) $(blmib-cflags) -o $@

#------------------------------------------------------------------

mk_test:$(target-outs) $(blmib-outs)
#mk_test:$(target-outs)

blmib:$(blmib-outs)

force:

clean:
	rm -f $(all-objs) $(target-outs) *.gcda *.gcno
	rm -f *.out

install:
	cp -av ./*.out $(prefix)/sixmid/ || exit 0
	cp -av ./*.so $(prefix)/sixmid/ || exit 0
#	cp -av ./*.pl $(prefix)/sixmid/ || exit 0


