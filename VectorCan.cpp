#include <boost/python/class.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/list.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/lvalue_from_pytype.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/errors.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/pointee.hpp>
#include <iostream>
#include <string>

#include "VectorCan.h"
#include "vxlapi.h"

using namespace vcan;
using namespace boost::python;

#define CHKERR(x) { \
		XLstatus xlStatus;\
		xlStatus = (x);\
		if (xlStatus != XL_SUCCESS) {\
            printf("Line: %d\n", __LINE__);\
			PyErr_SetString(PyExc_RuntimeError, xlGetErrorString(xlStatus));\
		}\
	}


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// CanMsg CLASS FOLOWS ////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

CanMsg::CanMsg(){
	id = 0;
	dlc = 0;
	time = 0;
}

CanMsg::CanMsg(s64 id, boost::python::list l, u32 flags) {
	this->id = id;
	this->flag = flags;
	this->dlc = 0;
	this->time = 0;
	
	u8 indx = 0;
	int nElem = boost::python::extract<int>(l.attr("__len__")());
	
	for (indx=0; indx < nElem && indx < 8; ++indx) {
		this->msg[indx] = boost::python::extract<int>( l[indx] );
	}
	
	this->dlc = indx;
}

CanMsg::~CanMsg(){
	rep.clear();
}

std::string CanMsg::__str__() {
	u8 i;
	char dt[64];
	char tmp[128];

	rep.clear();
	dt[0]='\0'; tmp[0]='\0';

	for (i=0; i<this->dlc;i++) {
		sprintf(dt+strlen(dt), "%X ", this->msg[i]);
	}
	sprintf(tmp,"CanMsg :: Id:0x%.8X Dlc:0x%.2X Flags: 0x%.4X Time: %.8X Data:[%s]", this->id,this->dlc,this->flag, this->time, dt);

	return rep.append(tmp);
}

boost::python::list CanMsg::getMsg(void){
	u8 indx = 0;
	boost::python::list l;
	
	for (indx=0; indx < dlc && indx < 8; ++indx) {
		l.append( msg[indx] );
	}
	
	return l;
}

void CanMsg::setMsg(boost::python::list l){
	u8 indx = 0;
	int nElem = boost::python::extract<int>(l.attr("__len__")());
	
	for (indx=0; indx < nElem && indx < 8; ++indx) {
		msg[indx] = boost::python::extract<int>( l[indx] );
	}
	
	dlc = indx;
}

   
//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// CAN CLASS FOLOWS ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
   
Can::Can() {
	channelsMask = 0;
	portHandle = XL_INVALID_PORTHANDLE;
	
	sprintf(appName,  "PyCanBridge");
	CHKERR(xlOpenDriver ());
	CHKERR(xlGetDriverConfig(&drvConfig));
}

Can::~Can(){
    //CHKERR(xlCanSetReceiveMode(portHandle, 0, 0));
	CHKERR(xlDeactivateChannel(portHandle, channelsMask));
	CHKERR(xlClosePort(portHandle));
	xlCloseDriver();
}


std::string Can:: __str__() {
	u8 i=0;
	u16 curI=0;
	char drvInfo[256];
	
	rep.clear();

	for (i=0; i < drvConfig.channelCount; i++) {
		drvInfo[0]='\0';
		sprintf(drvInfo, "Ch.: %02d, CM:0x%3I64x, %20s\n", 
				drvConfig.channel[i].channelIndex, 
				drvConfig.channel[i].channelMask, 
				drvConfig.channel[i].name);
		rep.append(drvInfo);
	}

	return rep;
}



void Can::openChannels(u32 channel_index_, u32 BitRate, u32 OutputMode)
{
    XLaccess permissionMask = 0;
    u32 channel = 0;
    u32 i = 0;
    int pelem = 0;
	std::ostringstream oss;
  
	
    XLchipParams chipParams = { 125000, 1, 4, 3, 1 };

    for (i = 0; i < drvConfig.channelCount; i++)
    {
        if ((drvConfig.channel[i].channelBusCapabilities & XL_BUS_COMPATIBLE_CAN) && drvConfig.channel[i].channelIndex == channel_index_)
        {
            channelsMask |= drvConfig.channel[i].channelMask;
            oss << "Channel is founded.\n";
            PySys_WriteStdout(oss.str().c_str());
        }
    }

	
    CHKERR(xlOpenPort( &portHandle, appName, channelsMask, &permissionMask, 256, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN));

    printf("PortHandle %d Invalid: %d\n", portHandle, XL_INVALID_PORTHANDLE);

    if (BitRate != 0)
    {
        CHKERR(xlCanSetChannelBitrate(portHandle, permissionMask, BitRate));

    }

    CHKERR(xlCanSetReceiveMode(portHandle, 1, 0));

    CHKERR(xlCanSetChannelOutput(portHandle, permissionMask, OutputMode));


    //CHKERR(xlCanSetChannelParams(portHandle, permissionMask, &chipParams));


    CHKERR(xlCanSetChannelTransceiver( portHandle, channelsMask, XL_TRANSCEIVER_TYPE_PB_CAN_SWC, //XL_TRANSCEIVER_TYPE_CAN_252,
                    XL_TRANSCEIVER_LINEMODE_SWC_NORMAL,//XL_TRANSCEIVER_LINEMODE_NORMAL ,
                    XL_TRANSCEIVER_RESNET_MASTER_STBY ));


    CHKERR(xlActivateChannel( portHandle, channelsMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK));

}

void Can::openChannels1(u32 channel_index){
	Can::openChannels(channel_index);
}

void Can::openChannels2(u32 channel_index,  u32 BitRate){
	Can::openChannels(channel_index, BitRate);
}

boost::python::list Can::read(){
	XLstatus        xlStatus;
	XLevent         xlEvent;
	unsigned int    msgsrx = 1;
	boost::python::list retList;
	//CanMsg  *tmp = NULL;
	CanMsg  tmp;
	u8 cI = 0;

	while(true){
		xlStatus = xlReceive(portHandle, &msgsrx, &xlEvent);
		if (xlStatus == XL_ERR_QUEUE_IS_EMPTY) 
			break;

		//tmp = new CanMsg;
		tmp.id = xlEvent.tagData.msg.id;
		tmp.dlc = xlEvent.tagData.msg.dlc;
		tmp.flag = xlEvent.tagData.msg.flags;
		tmp.time = (u64)xlEvent.timeStamp;
		
		for (cI=0; cI < 8; cI++){
			tmp.msg[cI] = 0;
			if (cI <= tmp.dlc) {
				tmp.msg[cI] = xlEvent.tagData.msg.data[cI];
			}
		}
		
		retList.append(tmp);
	}
	
	return retList;
}

int Can::write(boost::python::list inList){
		XLevent       xlEvent;
		XLstatus      xlstatus;
		u32 messageCount = 1;
		u8 celem=0;
		CanMsg msg;
		int pelem = 0;
		int nElem = extract<int>(inList.attr("__len__")());
		
		memset(&xlEvent, 0, sizeof(xlEvent));

		for (pelem = 0; pelem < nElem; ++pelem){
			msg = extract<CanMsg> ( inList[pelem] );
			
			xlEvent.tag    = XL_TRANSMIT_MSG;
			xlEvent.tagData.msg.id      = msg.id;
			xlEvent.tagData.msg.dlc     = msg.dlc;
			xlEvent.tagData.msg.flags   = msg.flag;
			
			for (celem = 0; celem < msg.dlc; celem++){
				xlEvent.tagData.msg.data[celem] = msg.msg[celem];
			}

            //printf("%d\n", channelsMask);
			xlstatus = xlCanTransmit(portHandle, channelsMask, &messageCount, &xlEvent);
		}
	return xlstatus;
}


enum CanMsgFlags {
	FLAG_ERROR_FRAME = XL_CAN_MSG_FLAG_ERROR_FRAME ,
	FLAG_OVERRUN = XL_CAN_MSG_FLAG_OVERRUN ,
	FLAG_NERR = XL_CAN_MSG_FLAG_NERR ,
	FLAG_WAKEUP = XL_CAN_MSG_FLAG_WAKEUP ,
	FLAG_REMOTE_FRAME = XL_CAN_MSG_FLAG_REMOTE_FRAME,
	FLAG_RESERVED_1 = XL_CAN_MSG_FLAG_RESERVED_1 ,
	FLAG_TX_COMPLETED = XL_CAN_MSG_FLAG_TX_COMPLETED ,
	FLAG_TX_REQUEST = XL_CAN_MSG_FLAG_TX_REQUEST
};

enum CanOutputFlags {
	OUTPUT_MODE_SILENT = XL_OUTPUT_MODE_SILENT,
	OUTPUT_MODE_NORMAL = XL_OUTPUT_MODE_NORMAL,
	OUTPUT_MODE_TX_OFF = XL_OUTPUT_MODE_TX_OFF,
//	OUTPUT_MODE_SJA_1000_SILENT = XL_OUTPUT_MODE_SJA_1000_SILENT
};

BOOST_PYTHON_MODULE(VectorCan)
{
    using namespace vcan;
	using namespace boost::python;
	
    class_<CanMsg>("CanMsg")
		.def(init<s64, boost::python::list, u32>())
        .def("__str__", &CanMsg::__str__)
        .add_property("msg", &CanMsg::getMsg , &CanMsg::setMsg)
		.def_readwrite("id", &CanMsg::id)
		.def_readonly("dlc", &CanMsg::dlc)
		.def_readwrite("time", &CanMsg::time)
        ;

     class_<Can>("Can")
        .def("__str__", &Can::__str__)
        .def("openChannels", &Can::openChannels)
		.def("openChannels", &Can::openChannels1)
		.def("openChannels", &Can::openChannels2)
		.def("read", &Can::read)
		.def("write", &Can::write)
		;
		
	enum_<CanMsgFlags>("CanMsgFlags") 
		.value("FLAG_ERROR_FRAME", FLAG_ERROR_FRAME)
		.value("FLAG_OVERRUN", FLAG_OVERRUN)
		.value("FLAG_NERR", FLAG_NERR)
		.value("FLAG_WAKEUP", FLAG_WAKEUP)
		.value("FLAG_REMOTE_FRAME", FLAG_REMOTE_FRAME)
		.value("FLAG_RESERVED_1", FLAG_RESERVED_1)
		.value("FLAG_TX_COMPLETED", FLAG_TX_COMPLETED)
		.value("FLAG_TX_REQUEST", FLAG_TX_REQUEST)
		;

	enum_<CanOutputFlags>("CanOutputFlags") 
		.value("OUTPUT_MODE_SILENT", OUTPUT_MODE_SILENT)
		.value("OUTPUT_MODE_NORMAL", OUTPUT_MODE_NORMAL)
		.value("OUTPUT_MODE_TX_OFF", OUTPUT_MODE_TX_OFF)
	//	.value("OUTPUT_MODE_SJA_1000_SILENT", OUTPUT_MODE_SJA_1000_SILENT)
		;
}
