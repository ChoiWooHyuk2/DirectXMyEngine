#include "AdapterReader.h"

std::vector<AdapterData> AdapterReader::adapters;


std::vector<AdapterData> AdapterReader::GetAdapters()
{
	if (adapters.size() > 0)// �̹� �ʱ�ȭ�� �Ǿ��ٸ� ��ȯ;
		return adapters;

	Microsoft::WRL::ComPtr<IDXGIFactory> pFactory;

	//DXGI ��ü ����
	HRESULT hr = CreateDXGIFactory(_uuidof(IDXGIFactory), reinterpret_cast<void**>(pFactory.GetAddressOf()));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "DXGI ���丮�� �������� ���߽��ϴ�.");
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
		ErrorLogger::Log(hr, "IDXGI �����Ϳ� ���� ������ �������� ���߽��ϴ�.");
	}
}

AdapterReader::AdapterReader()
{
}


AdapterReader::~AdapterReader()
{
}


