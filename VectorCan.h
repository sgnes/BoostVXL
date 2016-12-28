#ifndef _VCAN_CXX_H_
#define _VCAN_CXX_H_

#include <boost/python/class.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/list.hpp>
#include <windows.h>
#include <iostream>
#include <string>
#include <wtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "vxlapi.h"

namespace vcan { 
	typedef unsigned char  u8;
	typedef unsigned short u16;
	typedef unsigned int   u32;
	typedef unsigned long  u64;

	typedef signed char  s8;
	typedef signed short s16;
	typedef signed int   s32;
	typedef signed long  s64;

	class CanMsg{
		public:
			s64 id;
			u8 msg[8];
			u32 dlc;
			
			u32 flag;
			u64 time;

			CanMsg();
			CanMsg(s64 id, boost::python::list l, u32 flags);
			~CanMsg();
			
			std::string __str__();
			boost::python::list getMsg(void);
			void setMsg(boost::python::list );
		
		private:
			std::string rep;
	};

	class Can {
		private:
			XLdriverConfig  drvConfig;
			XLportHandle    portHandle;
			XLaccess 		  channelsMask;
			XLaccess 		  permMask;
			
			std::string rep;
			char appName[512];

		public:
			Can();
			~Can();

			std::string __str__();
			void openChannels(u32 channel_index_, u32 BitRate=0, u32 OutputMode=XL_OUTPUT_MODE_NORMAL);
			void openChannels1(u32 channel_index_);
			void openChannels2(u32 channel_index_, u32 BitRate=0);

			boost::python::list read();
			int write(boost::python::list);
	};
}


#endif
