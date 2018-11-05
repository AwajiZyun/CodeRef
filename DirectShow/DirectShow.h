#include "stdafx.h"
#include "Camera.h"
#include <iostream>

void getMediaSubtypeAsString(GUID type, char * typeAsString);

CCamera::CCamera()
: m_pMediaControl(NULL)
, m_pMediaEvent(NULL)
{
}


CCamera::~CCamera()
{
}

int CCamera::CameraCount()
{
    ::CoInitialize(NULL);
    int nCount = 0;

    // enumerate all video capture devices
    CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR){
        return nCount;
    }

    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK){
        nCount++;
    }

    pCreateDevEnum = NULL;
    pEm = NULL;
    ::CoUninitialize();
    return nCount;
}

bool CCamera::BindFilter(int nCameraIndex, IBaseFilter **pFilter)
{
    if (nCameraIndex < 0)
        return false;

    // enumerate all video capture devices
    ICreateDevEnum* pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR){
        return false;
    }

    CComPtr<IEnumMoniker> pEm;      // This will access the actual devices
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR){
        return false;
    }

    pEm->Reset();       // Go to the start of the enumerated list
    ULONG cFetched;
    IMoniker *pM;
    int index = 0;
    while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK, index <= nCameraIndex){
        if (index == nCameraIndex){
            IPropertyBag *pBag;
            // Binds to the storage for the specified object. Unlike the IMoniker::BindToObject method, 
            //      this method does not activate the object identified by the moniker.
            hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
            if (SUCCEEDED(hr)){
                VARIANT var;
                var.vt = VT_BSTR;
                hr = pBag->Read(L"FriendlyName", &var, NULL);
                if (hr == NOERROR){
                    // BindToObject将某个设备标识绑定到一个DirectShow Filter，
                    //     然后调用IFilterGraph::AddFilter加入到Filter Graph中，这个设备就可以参与工作了
                    // 调用IMoniker::BindToObject建立一个和选择的device联合的filter，
                    //      并且装载filter的属性(CLSID,FriendlyName, and DevicePath)。
                    // Binds to the specified object. The binding process involves finding the object, 
                    //      putting it into the running state if necessary, 
                    //      and providing the caller with a pointer to a specified interface on the identified object.
                    pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
                    SysFreeString(var.bstrVal);
                }
                pBag->Release();
            }
        }
        pM->Release();
        index++;
    }

    pCreateDevEnum->Release();
    return true;
}

bool CCamera::OpenCamera(int nCamID, bool bDisplayProperties/*=true*/, int nWidth /*=640*/, int nHeight /*=480*/)
{
    HRESULT hr = S_OK;
    ::CoInitialize(NULL);

    IGraphBuilder* pGraphBuilder = nullptr;
    IBaseFilter* pNullFilter = nullptr;
    IBaseFilter* pSampleGrabberFilter = nullptr;
    IBaseFilter* pDeviceFilter = nullptr;
    IPin* pCameraOutput = nullptr;
    IPin* pGrabberInput = nullptr;
    IPin* pGrabberOutput = nullptr;
    IPin* pNullInputPin = nullptr;

    //  Create the Filter Graph Manager. （用指定的类标识符创建一个Com对象）
    hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void**)&pGraphBuilder);
    if (FAILED(hr)){
        goto ERR;
    }
    // IMediaControl接口，用来控制流媒体在Filter Graph中的流动，例如流媒体的启动和停止；
    hr = pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
    if (FAILED(hr)){
        goto ERR;
    }
    // IMediaEvent接口，该接口在Filter Graph发生一些事件时用来创建事件的标志信息并传送给应用程序
    hr = pGraphBuilder->QueryInterface(IID_IMediaEvent, (void**)&m_pMediaEvent);
    if (FAILED(hr)){
        goto ERR;
    }
    hr = ::CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*)&pNullFilter);
    if (FAILED(hr)){
        goto ERR;
    }
    hr = pGraphBuilder->AddFilter(pNullFilter, L"NullRenderer");
    if (FAILED(hr)){
        goto ERR;
    }
    
    hr = ::CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pSampleGrabberFilter);
    if (FAILED(hr)){
        goto ERR;
    }

    // 查询得到组件对象上的接口
    hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);
    if (FAILED(hr)){
        goto ERR;
    }

    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;
    hr = m_pSampleGrabber->SetMediaType(&mt);
    if (FAILED(hr)){
        goto ERR;
    }

    MYFREEMEDIATYPE(mt);

    hr = pGraphBuilder->AddFilter(pSampleGrabberFilter, L"SampleGrab");
    if (FAILED(hr)){
        goto ERR;
    }

    // Bind Device Filter.  We know the device because the id was passed in
    BindFilter(nCamID, &pDeviceFilter);
    hr = pGraphBuilder->AddFilter(pDeviceFilter, NULL);
    if (FAILED(hr)){
        goto ERR;
    }

    IEnumPins* pEnum;
    pDeviceFilter->EnumPins(&pEnum);
    hr = pEnum->Reset();    // The Reset method resets the enumeration sequence to the beginning.
    
    hr = pEnum->Next(1, &pCameraOutput, NULL);    // The Next method retrieves a specified number of pins in the enumeration sequence.
    pEnum->Release();

    pSampleGrabberFilter->EnumPins(&pEnum);
    pEnum->Reset();
    hr = pEnum->Next(1, &pGrabberInput, NULL);
    pEnum->Release();
    pSampleGrabberFilter->EnumPins(&pEnum);
    pEnum->Reset();
    pEnum->Skip(1);
    
    hr = pEnum->Next(1, &pGrabberOutput, NULL);
    pEnum->Release();

    pNullFilter->EnumPins(&pEnum);
    pEnum->Reset();

    hr = pEnum->Next(1, &pNullInputPin, NULL);
    pEnum->Release();

    // Show camera property pages
    if (bDisplayProperties){
        ISpecifyPropertyPages* pPages;

        HRESULT hr = pCameraOutput->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);
        if (SUCCEEDED(hr)){
            PIN_INFO PinInfo;
            pCameraOutput->QueryPinInfo(&PinInfo);

            CAUUID caGUID;
            pPages->GetPages(&caGUID);

            OleCreatePropertyFrame(NULL, 0, 0, L"Property Sheet", 1, (IUnknown**)&(pCameraOutput),
                caGUID.cElems, caGUID.pElems, 0, 0, NULL);
            CoTaskMemFree(caGUID.pElems);
            PinInfo.pFilter->Release();
        }
        pPages->Release();
        pPages = NULL;
    }
    else{
        IAMStreamConfig* iconfig = NULL;
        hr = pCameraOutput->QueryInterface(IID_IAMStreamConfig, (void**)&iconfig);

        AM_MEDIA_TYPE* pmt;
        if (iconfig->GetFormat(&pmt) != S_OK){
            return false;
        }

        VIDEOINFOHEADER* phead;
        if (pmt->formattype == FORMAT_VideoInfo){
            pmt->subtype = MEDIASUBTYPE_MJPG/*MEDIASUBTYPE_RGB24*/;
            phead = (VIDEOINFOHEADER*)pmt->pbFormat;
            phead->bmiHeader.biWidth = nWidth;
            phead->bmiHeader.biHeight = nHeight;
            if ((hr = iconfig->SetFormat(pmt)) != S_OK){
                return false;
            }
        }

        iconfig->Release();
        MYFREEMEDIATYPE(*pmt);
    }

    hr = pGraphBuilder->Connect(pCameraOutput, pGrabberInput);
    if (FAILED(hr)){
        goto ERR;
    }
    hr = pGraphBuilder->Connect(pGrabberOutput, pNullInputPin);
    if (FAILED(hr)){
        goto ERR;
    }

    // The SetBufferSamples method specifies whether to copy sample data into a buffer as it goes through the filter.
    m_pSampleGrabber->SetBufferSamples(TRUE);
    // The SetOneShot method specifies whether the Sample Grabber filter halts after the filter receives a sample.
    m_pSampleGrabber->SetOneShot(FALSE);
    hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
    if (FAILED(hr)){
        goto ERR;
    }

    VIDEOINFOHEADER *videoHeader;
    videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
    m_nWidth = videoHeader->bmiHeader.biWidth;
    m_nHeight = videoHeader->bmiHeader.biHeight;
    m_bConnected = true;

    m_pMediaControl->Run();
    bRunning = true;

    pGraphBuilder->Release();
    pDeviceFilter->Release();
    pSampleGrabberFilter->Release();
    pGrabberInput->Release();
    pGrabberOutput->Release();
    pCameraOutput->Release();
    pNullFilter->Release();
    pNullInputPin->Release();
    ::CoUninitialize();
    return true;

ERR:
    if (pGraphBuilder){
        pGraphBuilder->Release();
    }
    if (m_pMediaControl) {
        m_pMediaControl->Release();
        m_pMediaControl = nullptr;
    }
    if (m_pMediaEvent) {
        m_pMediaEvent->Release();
        m_pMediaEvent = nullptr;
    }
    if (pDeviceFilter){
        pDeviceFilter->Release();
    }
    if (pSampleGrabberFilter){
        pSampleGrabberFilter->Release();
    }
    if (m_pSampleGrabber) {
        m_pSampleGrabber->Release();
        m_pSampleGrabber = nullptr;
    }
    if (pGrabberInput) {
        pGrabberInput->Release();
    }
    if (pGrabberOutput) {
        pGrabberOutput->Release();
    }
    if (pCameraOutput) {
        pCameraOutput->Release();
    }
    if (pNullFilter) {
        pNullFilter->Release();
    }
    if (pNullInputPin) {
        pNullInputPin->Release();
    }

    ::CoUninitialize();
    return false;
}


CImage* CCamera::QueryFrame()
{
    //m_pMediaControl->Run();
    //long evCode;
    //m_pMediaEvent->WaitForCompletion(INFINITE, &evCode);

    long size = 0;
    HRESULT hr = m_pSampleGrabber->GetCurrentBuffer(&size, NULL);    // If pBuffer is NULL, this parameter receives the required buffer size
    if (FAILED(hr)){
        //m_image.Destroy();
        return NULL;
    }
    
    // if the buffer size changed
    if (size != m_nBufferSize){
        m_nBufferSize = size;
        m_image.Destroy();
        m_image.Create(m_nWidth, m_nHeight, 24);
    }
    if (m_image.IsNull()){
        return NULL;
    }
    byte* q = NULL;
    byte* p = new byte[m_nWidth * m_nHeight * 3];
    m_pSampleGrabber->GetCurrentBuffer(&m_nBufferSize, (long*)p);   // If pBuffer is not NULL, set this parameter equal to the size of the buffer, in bytes.

    for (int y = 0, z = m_nHeight - 1; y < m_nHeight, z >= 0; y++, z--){
        q = (byte*)m_image.GetPixelAddress(0, z);
        memcpy(q, &p[m_nWidth * 3 * y], m_nWidth * 3);
    }

    delete[] p;
    return &m_image;
}

int CCamera::PauseRestart(bool pause)
{
    if (m_pMediaControl){
        if (pause){
            m_pMediaControl->Pause();
        }
        else{
            m_pMediaControl->Run();
        }
    }
    else{
        return -1;
    }

    return 0;
}

bool CCamera::CloseCamera()
{
    bRunning = false;
    if (m_pMediaControl) {
        m_pMediaControl->Stop(); 
        m_pMediaControl->Release();
        m_pMediaControl = nullptr;
    }

    if (m_pSampleGrabber) {
        m_pSampleGrabber->Release();
        m_pSampleGrabber = nullptr;
    }
    
    if (m_pMediaEvent) {
        m_pMediaEvent->Release();
        m_pMediaEvent = nullptr;
    }

    return true;
}

std::vector<POINT> CCamera::GetAllSupportPix(int iDeviceID)
{
    HRESULT hr = S_OK;
    std::vector<POINT> vecPix;
    ICaptureGraphBuilder2* _pCapture = NULL;
    IBaseFilter* _pBF;

    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
        IID_ICaptureGraphBuilder2, (void **)&_pCapture);
    if (FAILED(hr))
        return vecPix;

    if (!BindFilter(iDeviceID, &_pBF)){
        _pCapture->Release();
        return vecPix;
    }
    IAMStreamConfig*  pStreamConfig;
    AM_MEDIA_TYPE *pmtConfig;
    hr = _pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, 0, _pBF, IID_IAMStreamConfig, (void**)&pStreamConfig);
    if (FAILED(hr)){
        _pCapture->Release();
        _pBF->Release();
        return vecPix;
    }
    pStreamConfig->GetFormat(&pmtConfig);
    if (FAILED(hr)){
        _pCapture->Release();
        _pBF->Release();
        pStreamConfig->Release();
        return vecPix;
    }
    VIDEOINFOHEADER * vi = (VIDEOINFOHEADER*)pmtConfig->pbFormat;

    int iCount, iSize;
    VIDEO_STREAM_CONFIG_CAPS caps;
    pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);

    AM_MEDIA_TYPE *pmtPV = NULL;
    for (int i = 0; i<iCount; ++i)
    {
        if (pStreamConfig->GetStreamCaps(i, &pmtPV, (BYTE*)&caps) == S_OK)
        {
            if (pmtPV->subtype == MEDIASUBTYPE_RGB24)
            {
                POINT pix = { caps.MaxOutputSize.cx, caps.MaxOutputSize.cy };
                vecPix.push_back(pix);
            }
            //FreeMediaType(*pmtPV);
        }
    }

    _pCapture->Release();
    _pBF->Release();
    //FreeMediaType(*pmtConfig);
    pStreamConfig->Release();
    return vecPix;
}

int CCamera::Capture()
{
    return 0;
}

static const GUID GUID_GPIO_CTL = {
    0x46394292,
    0x0CD1,
    0x4AE3,
    0x87, 0x83, 0x31, 0x33, 0xF9, 0xEA, 0xAA, 0x3B };

typedef struct {
    KSPROPERTY      Property;
    ULONG           NodeId;
    ULONG           Reserved;
} KSP_NODE, *PKSP_NODE;
bool CCamera::OpenLED(long lData)
{
    IBaseFilter* pBF = NULL;
    BindFilter(0, &pBF);

    IKsControl *pCtl = NULL;
    HRESULT hr = pBF->QueryInterface(IID_IKsControl, (void **)&pCtl);
    if (FAILED(hr)) {
        return false;
    }

    KSP_NODE knod = { 0 };
    long  nEffects1 = 0x0000d160;
    long  nEffects2 = 0x00000002;
    long  nEffects3 = 0x00000207;
    long  nEffects4 = (1 == lData) ? 0x000000fe : 0x0000007e;
    ULONG dwRsz = 0;

    knod.Property.Set = GUID_GPIO_CTL;
    knod.Property.Id = 1;
    knod.Property.Flags = (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY);
    knod.NodeId = 1;
    knod.Reserved = 0;

    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects1, sizeof(nEffects1), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects2, sizeof(nEffects2), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects3, sizeof(nEffects3), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects4, sizeof(nEffects4), &dwRsz);

    pCtl->Release();
    return true;
}

bool CCamera::OpenBuzzer(long lData)
{
    IBaseFilter* pBF = NULL;
    BindFilter(0, &pBF);

    IKsControl *pCtl = NULL;
    HRESULT hr = pBF->QueryInterface(IID_IKsControl, (void **)&pCtl);
    if (FAILED(hr)) {
        return false;
    }

    KSP_NODE knod = { 0 };
    long  nEffects1 = 0x0000d160;
    long  nEffects2 = 0x00000002;
    long  nEffects3 = 0x00000207;
    long  nEffects4 = 0;    // (1 == lData) ? 0x000000fe : 0x0000007e;
    ULONG dwRsz = 0;

    knod.Property.Set = GUID_GPIO_CTL;
    knod.Property.Id = 1;
    knod.Property.Flags = (KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY);
    knod.NodeId = 1;
    knod.Reserved = 0;

    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects1, sizeof(nEffects1), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects2, sizeof(nEffects2), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects3, sizeof(nEffects3), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects4, sizeof(nEffects4), &dwRsz);
    
    nEffects1 = 0x0000d160;
    nEffects2 = 0x00000002;
    nEffects3 = 0x00000207;
    if (1 == lData){
        nEffects4 |= 0x00000001;
    }
    else{
        nEffects4 &= 0xFFFFFFFE;
    }
    knod.Property.Flags = (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects1, sizeof(nEffects1), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects2, sizeof(nEffects2), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects3, sizeof(nEffects3), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects4, sizeof(nEffects4), &dwRsz);

    pCtl->Release();
    return true;
}

bool CCamera::IsButtonDown()
{

    IBaseFilter* pBF = NULL;
    BindFilter(0, &pBF);

    IKsControl *pCtl = NULL;
    HRESULT hr = pBF->QueryInterface(IID_IKsControl, (void **)&pCtl);
    if (FAILED(hr)) {
        return false;
    }

    KSP_NODE knod = { 0 };
    long  nEffects1 = 0x0000d160;
    long  nEffects2 = 0x00000002;
    long  nEffects3 = 0x00000207;
    long  nEffects4 = 0;
    ULONG dwRsz = 0;

    knod.Property.Set = GUID_GPIO_CTL;
    knod.Property.Id = 1;
    knod.Property.Flags = (KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY);
    knod.NodeId = 1;
    knod.Reserved = 0;

    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects1, sizeof(nEffects1), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects2, sizeof(nEffects2), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects3, sizeof(nEffects3), &dwRsz);
    hr = pCtl->KsProperty(&knod.Property, sizeof(knod), &nEffects4, sizeof(nEffects4), &dwRsz);

    // Wrong!
    if (nEffects4 & 0x10){
        std::cout << "nEffect4:" << std::hex << nEffects4 << " UP" << std::endl;
        return false;
    }
    std::cout << "nEffect4:" << std::hex << nEffects4 << " DOWN" << std::endl;
    return true;
}