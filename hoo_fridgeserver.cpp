#include <chrono>
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;

namespace PH = std::placeholders;

class Resource
{
    protected:
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_rep;
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)=0;
};

class DeviceResource : public Resource
{
    public:

    DeviceResource():m_modelName{}
    {
        std::string resourceURI = "/device";
        std::string resourceTypeName = "intel.fridge";
        std::string resourceInterface = DEFAULT_INTERFACE;
        EntityHandler cb = std::bind(&DeviceResource::entityHandler, this,PH::_1);

        uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
            resourceURI, resourceTypeName, resourceInterface, cb,
            resourceProperty);

        if(OC_STACK_OK != result)
        {
            throw std::runtime_error(
                std::string("Device Resource failed to start")+std::to_string(result));
        }
    }

    OCResourceHandle getHandle()
    {
        return m_resourceHandle;
    }

    OCRepresentation get()
    {
	     m_rep.setValue("device_name", std::string("Intel Powered 2 door, 1 light refrigerator"));
       return m_rep;
    }

    void put(const OCRepresentation& rep)
    {
        try
        {
            std::cout << "In put()" << std::endl;
            if (rep.getValue("Message", m_msg))
            {
                std::cout << "\t\t" << "Received Message: " << m_msg << std::endl;
            }
        }
        catch (exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    private:
    std::string m_modelName;
    std::string m_msg;

    protected:
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if(request)
        {
            QueryParamsMap queries = request->getQueryParameters();
            auto requestRepresentation = request->getResourceRepresentation();
            std::cout << "++++++++++++++++++++++++++++++++++++++++++++++\n";
            std::cout << "Getting query params destined for: " << request->getResourceUri()
                  << ", Type " << request->getRequestType() << std::endl ;
            for(auto it = queries.cbegin(); it != queries.cend(); it++)
            {
                std::cout << it->first << " : " << it->second << std::endl;
            }
            std::cout << "++++++++++++++++++++++++++++++++++++++++++++++\n";

            if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
            {
                auto pResponse = std::make_shared<OC::OCResourceResponse>();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                if(request->getRequestType() == "GET")
                {
                    std::cout<<"DeviceResource Get Request"<<std::endl;
                    pResponse->setErrorCode(200);
                    pResponse->setResponseResult(OC_EH_OK);
                    pResponse->setResourceRepresentation(get(), "");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                            ehResult = OC_EH_OK;
                    }
                }
                else if(request->getRequestType() == "PUT")
                {
                    std::cout<<"DeviceResource Post Request"<<std::endl;
	                  put(request->getResourceRepresentation());
                    pResponse->setErrorCode(200);
	                  pResponse->setResponseResult(OC_EH_OK);

                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else
                {
                    std::cout <<"DeviceResource unsupported request type "
                    << request->getRequestType() << std::endl;
                    pResponse->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(pResponse);
                    ehResult = OC_EH_ERROR;
                }
            }
            else
            {
                std::cout << "DeviceResource unsupported request flag" <<std::endl;
            }
        }

        return ehResult;
    }
};

class LightResource : public Resource
{
    public:
    LightResource() : m_isOn(false)
    {
        std::string resourceURI = "/light";
        std::string resourceTypeName = "intel.fridge.light";
        std::string resourceInterface = DEFAULT_INTERFACE;
        EntityHandler cb = std::bind(&LightResource::entityHandler, this,PH::_1);

        uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
            resourceURI, resourceTypeName, resourceInterface, cb,
            resourceProperty);

        if(OC_STACK_OK != result)
        {
            throw std::runtime_error(
                std::string("Light Resource failed to start")+std::to_string(result));
        }
    }

    OCRepresentation get()
    {
       m_rep.setValue("light state", std::string("off"));
       return m_rep;
    }

    void put(const OCRepresentation& rep)
    {
        try
        {
            std::cout << "In Light::put()" << std::endl;
            if (rep.getValue("Message", m_lightmsg))
            {
                std::cout << "\t\t" << "Received Message: " << m_lightmsg << std::endl;
            }
        }
        catch (exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    private:
    bool m_isOn;
    std::string m_lightmsg;

    protected:
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if(request)
        {
            std::cout << "In entity handler for Light, URI is : "
                      << request->getResourceUri() << std::endl;

            if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
            {
                auto pResponse = std::make_shared<OC::OCResourceResponse>();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                if(request->getRequestType() == "GET")
                {
                    std::cout<<"Light Get Request"<<std::endl;
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(get(), "");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if(request->getRequestType() == "PUT")
                {
                    std::cout <<"Light Put Request"<<std::endl;
		                put(request->getResourceRepresentation());
                    pResponse->setErrorCode(200);
                    if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else
                {
                    std::cout << "Light unsupported request type"
                    << request->getRequestType() << std::endl;
                    pResponse->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(pResponse);
                    ehResult = OC_EH_ERROR;
                }
            }
            else
            {
                std::cout << "Light unsupported request flag" <<std::endl;
            }
        }

        return ehResult;
    }
};

class DoorResource : public Resource
{
    public:
    DoorResource():m_isOpen{false}
    {

        std::string resourceURI = "/door";
        std::string resourceTypeName = "intel.fridge.door";
        std::string resourceInterface = DEFAULT_INTERFACE;
        EntityHandler cb = std::bind(&DoorResource::entityHandler, this,PH::_1);

        uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
            resourceURI, resourceTypeName, resourceInterface, cb,
            resourceProperty);

        if(OC_STACK_OK != result)
        {
            throw std::runtime_error(
                std::string("Door Resource failed to start")+std::to_string(result));
        }
    }

    OCRepresentation get()
    {
        m_rep.setValue("door state", std::string("open"));
        return m_rep;
    }

    void put(const OCRepresentation& rep)
    {
        try
        {
            std::cout << "In Door::put()" << std::endl;
            if (rep.getValue("Message", m_doormsg))
            {
                std::cout << "\t\t" << "Received Message: " << m_doormsg << std::endl;
            }
        }
        catch (exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    private:
    bool m_isOpen;
    std::string m_doormsg;

    protected:
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        std::cout << "EH of door invoked " << std::endl;
        OCEntityHandlerResult ehResult = OC_EH_ERROR;

        if(request)
        {
            std::cout << "In entity handler for Door, URI is : "
                      << request->getResourceUri() << std::endl;

            if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
            {
                auto pResponse = std::make_shared<OC::OCResourceResponse>();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                if(request->getRequestType() == "GET")
                {
                    // note that we know the side because std::bind gives us the
                    // appropriate object
                    std::cout<< " Door Get Request"<<std::endl;
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(get(), "");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if(request->getRequestType() == "PUT")
                {
                    std::cout << " Door Put Request"<<std::endl;
                    put(request->getResourceRepresentation());
                    pResponse->setErrorCode(200);
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else
                {
                    std::cout << " Door unsupported request type "
                    << request->getRequestType() << std::endl;
                    pResponse->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(pResponse);
                    ehResult = OC_EH_ERROR;
                }
            }
            else
            {
                std::cout << "Door unsupported request flag" <<std::endl;
            }
        }

        return ehResult;
    }

};

class Refrigerator
{
    public:
    Refrigerator()
        :
        m_device(),
        m_light(),
        m_maindoor()
    {
    }
    private:
    DeviceResource m_device;
    LightResource m_light;
    DoorResource m_maindoor;
};

int main ()
{
    PlatformConfig cfg
    {
        ServiceType::InProc,
        ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);
    Refrigerator rf;

    // we will keep the server alive for at most 30 minutes
    std::this_thread::sleep_for(std::chrono::minutes(30));
    return 0;
}
