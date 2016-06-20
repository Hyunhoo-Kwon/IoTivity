//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

///
///This sample demonstrates the device discovery feature
///The client queries for the device related information
///stored by the server.
///

#include <mutex>
#include <condition_variable>
#include <iostream>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;

typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;
DiscoveredResourceMap discoveredResources;

std::mutex resourceLock;


// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    using namespace OC::OCPlatform;
    std::lock_guard<std::mutex> lock(resourceLock);

    std::string resourceURI;

    try
    {
        // Do some operations with resource object.
        if(resource)
        {
            std::vector<int>::iterator it;

            std::cout<<"DISCOVERED Resource:"<<std::endl;
            // Get the resource URI
            resourceURI = resource->uri();

            std::cout << "Discovered a device object"<<std::endl;
            if(discoveredResources.find(resource->uniqueIdentifier()) == discoveredResources.end())
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() <<std::endl;
                discoveredResources.insert(std::pair<OCResourceIdentifier,
                std::shared_ptr<OCResource>> (resource->uniqueIdentifier(), resource));
                // discoveredResources[resource->uniqueIdentifier()] = resource;
                /*g_ResURI.push_back(resource->uri());*/
                std::cout << "\tHost: "<<resource->host()<<std::endl;
                std::cout << "\tURI:  "<<resource->uri() <<std::endl;
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Exception caught in Found Resource: "<< e.what() <<std::endl;
    }
}


void receivedDeviceInfo(const OCRepresentation& rep)
{
    std::cout << "\nDevice Information received ---->\n";
    std::string value;
    std::string values[] =
    {
        "di",   "Device ID        ",
        "n",    "Device name      ",
        "lcv",  "Spec version url ",
        "dmv",  "Data Model Model ",
    };

    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]) ; i += 2)
    {
        if(rep.getValue(values[i], value))
        {
            std::cout << values[i + 1] << " : "<< value << std::endl;
        }
    }
    std::cout << "\n";
}

void printDevInfo(void)
{
    std::shared_ptr<OCResource> res;
    std::map<OCResourceIdentifier, std::shared_ptr<OCResource>>::iterator i;

    for(i =discoveredResources.begin();i != discoveredResources.end(); ++i)
    {
        res = (*i).second;
        cout << "Resource URI = " << res->uri()
        << "\t HostID = " << res->host() << endl;
    }
}

// callback handler on PUT request
void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{

    if(eCode == 0)
    {
        std::cout << "PUT request was successful" << std::endl;
    }
    else
    {
        std::cout << "onPut Response error: " << eCode << std::endl;
        std::exit(-1);
    }

}


int main(int argc, char* argv[]) {

    std::ostringstream platformDiscoveryRequest;
    std::ostringstream deviceDiscoveryRequest;
    std::ostringstream requestURI;

    std::string platformDiscoveryURI = "/oic/p";
    std::string deviceDiscoveryURI   = "/oic/d";

    //Default Connectivity type
    OCConnectivityType connectivityType = CT_ADAPTER_IP;
    bool isRun = true;

    std::cout << "Usage devicediscoveryclient\n" << std::endl;

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Client,
        "0.0.0.0",
        0,
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);
    try
    {
        deviceDiscoveryRequest << OC_MULTICAST_PREFIX << deviceDiscoveryURI;

        OCStackResult ret;
        std::cout<< "Querying for device information... ";

	// Find all devices
        ret = OCPlatform::getDeviceInfo("", deviceDiscoveryRequest.str(), connectivityType,
                &receivedDeviceInfo);

        if (ret == OC_STACK_OK)
        {
            std::cout << "done." << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }

	// Find all resources
        requestURI << OC_RSRVD_WELL_KNOWN_URI;

        OCPlatform::findResource("", requestURI.str(), connectivityType, &foundResource);
        

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        //cv.wait(lock, []{return false;});
	cv.wait_for(lock, std::chrono::seconds(3));

	while (isRun)
        {
            int selectedMenu;

            cout << endl <<  "0 :: Quit \n1 :: List All Discovered Devices ";
            cout << "\n2 :: Refresh List of Devices " << endl;
	    cout << "3 :: Send Text Msg to Device" << endl;

            cin >> selectedMenu;
            OCRepresentation rep;

            switch(selectedMenu)
            {
                case 0:
                    isRun = false;
                break;
                case 1:
                {
		    cout << "Listing all the Discovered Devices..." << endl;
                    printDevInfo();
                }
                break;

                case 2:
                {
		    cout << "Refreshing the List of Discovered Devices..." << endl;
                    discoveredResources.clear();

		    std::cout<< "Querying for device information... ";

		    // Find all devices
        	    ret = OCPlatform::getDeviceInfo("", deviceDiscoveryRequest.str(), connectivityType,
                	&receivedDeviceInfo);

        	    if (ret == OC_STACK_OK)
        	    {
            		std::cout << "done." << std::endl;
        	    }
        	    else
        	    {
            		std::cout << "failed." << std::endl;
        	    }

		    // Find all resources
                    OCPlatform::findResource("", requestURI.str(), connectivityType, &foundResource);
                    std::cout<< "Refreshing Resources... " <<std::endl;
                    cv.wait_for(lock, std::chrono::seconds(3));
                }
                break;
                case 3:
                {
                    std::string mesg;

                    std::shared_ptr<OCResource> res;
                    std::map<OCResourceIdentifier, std::shared_ptr<OCResource>>::iterator i;

		    cout << "\tSending Text Message to a Device..." << endl;
		    for(i =discoveredResources.begin();i != discoveredResources.end(); ++i)
                    {

                      	res = (*i).second;
			cout << res->uri() << "\n";
		    }

		    cout << "Enter the Message: ";
                    cin >> mesg;
			OCRepresentation rep;
			rep.setValue("Message", mesg);

		     cout << "Send Text Message to All Resource: \n";


		     // Do a Post/Put
                     for(i =discoveredResources.begin();i != discoveredResources.end(); ++i)
                     {

                        res = (*i).second;
		

			cout << "Sending Message ( " << mesg << " ) "
                               << " to \n Host = " << res->host()
                               << " Resource = " << res->uri() << endl;

 
			// Put message
			res->put(rep, QueryParamsMap(), &onPut);
                     }

                }
                break;
                default:
                    cout << "Invalid option" << endl;
                    break;
            }
            fflush(stdin);	
	}


    }catch(OCException& e)
    {
        std::cerr << "Failure in main thread: "<<e.reason()<<std::endl;
    }

    return 0;
}


