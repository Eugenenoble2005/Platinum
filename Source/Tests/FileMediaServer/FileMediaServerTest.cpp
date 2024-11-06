/*****************************************************************
|
|   Platinum - Test UPnP A/V MediaServer
|
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
| 
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltUPnP.h"
#include "PltFileMediaServer.h"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <stdlib.h>
#include <thread>

NPT_SET_LOCAL_LOGGER("platinum.media.server.file.test")

/*----------------------------------------------------------------------
|   PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, "usage: tinyupnp  <path>\n");
    fprintf(stderr, "<path> : Local path to serve. Leave blank to serve current working directory \n");
    exit(1);
}

/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
std::atomic<bool> keepRunning = true; 
void signal_handler(int signal){
    if(signal == SIGINT || signal == SIGTERM){
        keepRunning = false;
    }
}int main(int /* argc */, char** argv)
{
    // setup Neptune logging
    //NPT_LogManager::GetDefault().Configure("plist:.level=FINE;.handlers=ConsoleHandler;.ConsoleHandler.colors=off;.ConsoleHandler.filter=56");

    /* parse command line */
    char * provided_path = argv[1];
    const char * true_path = provided_path == nullptr ? "." : provided_path;     
    /* for faster DLNA faster testing */
    PLT_Constants::GetInstance().SetDefaultDeviceLease(NPT_TimeInterval(60.));
    PLT_UPnP upnp;
       PLT_DeviceHostReference device(
        new PLT_FileMediaServer(
            true_path, 
            "TinyUPnP Media Server",
            false,
            NULL, // NULL for random ID
            (NPT_UInt16)0)
            );

    NPT_List<NPT_IpAddress> list;
    NPT_CHECK_SEVERE(PLT_UPnPMessageHelper::GetIPAddresses(list));
    NPT_String ip = list.GetFirstItem()->ToString();

    device->m_ModelDescription = "TinyUPnP";
    device->m_ModelURL = "http://www.github.com/eugenenoble2005/tinyupnp.git";
    device->m_ModelNumber = "1.0";
    device->m_ModelName = "TinyUPnp Media Server";
    device->m_Manufacturer = "Noble Eugene";
    device->m_ManufacturerURL = "https://www.github.com/eugenenoble2005/tinyupnp.git";

    upnp.AddDevice(device);
    NPT_String uuid = device->GetUUID();
    fprintf(stdout, "Starting Server \r\n");
    NPT_CHECK_SEVERE(upnp.Start());
    fprintf(stdout, "Server Running. \r\n");
    char buf[256];

    std::signal(SIGINT,signal_handler);   
    
     while (keepRunning) {     
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    fprintf(stdout, "Killing Server" );
    upnp.Stop();
    fprintf(stdout, "Server Killed. Bye Bye \r\n");
    return 0;
}
