#include "AdapterReader.h"

std::vector<AdapterData> AdapterReader::adapters;


std::vector<AdapterData> AdapterReader::GetAdapters()
{
	if (adapters.size() > 0)// 이미 초기화가 되었다면 반환;
		return adapters;

	Microsoft::WRL::ComPtr<IDXGIFactory> pFactory;

	//DXGI 객체 생성
	HRESULT hr = CreateDXGIFactory(_uuidof(IDXGIFactory), reinterpret_cast<void**>(pFactory.GetAddressOf()));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "DXGI 팩토리를 생성하지 못했습니다.");
	}

	IDXGIAdapter* pAdapter;
	UINT index = 0;
	while (SUCCEEDED(pFactory->EnumAdapters(index, &pAdapter)))
	{
		adapters.push_back(AdapterData(pAdapter));
		index += 1;
	}
	return adapters;
}

AdapterData::AdapterData(IDXGIAdapter * pAdapter)
{
	this->pAdapter = pAdapter;
	HRESULT hr = pAdapter->GetDesc(&this->description);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "IDXGI 어탭터에 대한 정보를 가져오지 못했습니다.");
	}
}

AdapterReader::AdapterReader()
{
}


AdapterReader::~AdapterReader()
{
}


