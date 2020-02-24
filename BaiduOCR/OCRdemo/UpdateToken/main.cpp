#include <QtCore/QCoreApplication>
#include "RequestInterface.h"


//https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=uWeAsnBl5EjrOYaVaikIvBu1&client_secret=c2Shjk37rISOPQoQlaLa1Vu3VXT9KO45


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    RequestInterface request(nullptr);
    request.postRequest();

    return a.exec();
}
