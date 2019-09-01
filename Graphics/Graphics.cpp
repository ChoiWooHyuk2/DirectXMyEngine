#include "Graphics.h"



bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;
	this->fpsTimer.Start();

	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	//ImGui ����
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceConstext.Get());
	ImGui::StyleColorsDark();

	return true;
}

void Graphics::RenderFrame()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f};//������
	this->deviceConstext->ClearRenderTargetView(this->rederTargetView.Get(), bgcolor);
	this->deviceConstext->ClearDepthStencilView(this->depthStencilView.Get(),D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,0);

	this->deviceConstext->IASetInputLayout(this->vertexshader.GetInputLayout());
	this->deviceConstext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->deviceConstext->RSSetState(this->rasterizerState.Get());
	
	this->deviceConstext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	this->deviceConstext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());

	this->deviceConstext->VSSetShader(vertexshader.GetShader(),NULL,0);
	this->deviceConstext->PSSetShader(pixelshader.GetShader(),NULL,0);

	UINT offset = 0;

	//��� ���� ������Ʈ
	static float translationOffset[3] = {0,0,0};
	XMMATRIX world = XMMatrixTranslation(translationOffset[0], translationOffset[1], translationOffset[2]);
	constantBuffer.data.mat = world * camera.GetViewMatrix() * camera.GetProjectionMatrix();
	constantBuffer.data.mat = DirectX::XMMatrixTranspose(constantBuffer.data.mat);

	if (!constantBuffer.ApplyChanges())
		return;
	this->deviceConstext->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());

	//�簢��
	this->deviceConstext->PSSetShaderResources(0, 1, this->myTexture.GetAddressOf());
	this->deviceConstext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), vertexBuffer.StridePtr(), &offset);
	this->deviceConstext->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	this->deviceConstext->DrawIndexed(indicesBuffer.BufferSize(), 0, 0);

	//�۾� �׸���
	static int fpsCounter = 0;
	static std::string fpsString = "FPS : 0";
	fpsCounter += 1;
	if (fpsTimer.GetMilisecondsElapsed() > 1000.0)
	{
		fpsString = "FPS :" + std::to_string(fpsCounter);
		fpsCounter = 0;
		fpsTimer.Restart();
	}
	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), StringConverter::StringToWide(fpsString).c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f,  DirectX::XMFLOAT2(0.0f,0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();


	static int counter = 0;
	//ImGui ���� 
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//ImGui �׽�Ʈ ������ ����
	ImGui::Begin("Test");
	ImGui::Text("test text");
	if (ImGui::Button("CLICK"))
	counter += 1;
	ImGui::SameLine();
	std::string clickCount = "Click Count: " + std::to_string(counter);
	ImGui::Text(clickCount.c_str());
	ImGui::DragFloat3("Translation X/Y/Z", translationOffset, 0.1f, -5.0f, 5.0f);
	ImGui::End();
	//�Բ� �׸� ������ ����
	ImGui::Render();
	//������ �׸���
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	this->swapchain->Present(0, NULL);//��������ȭ ���ΰ� 0, 1�� �Ǿ�����
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		ErrorLogger::Log("IDXGI ��͸� ã�� �� �����ϴ�.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc.Width = this->windowWidth;
	scd.BufferDesc.Height = this->windowHeight;
	scd.BufferDesc.RefreshRate.Numerator = 60; //��� ���� �뷫 1�ʿ� 60FPS ��¶�
	scd.BufferDesc.RefreshRate.Denominator = 1; //�и�
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //���ҽ������� ����
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //��ĵ���� ����
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //�־��� ����� �ػ󵵿� �°� �̹����� �þ�� ���

	scd.SampleDesc.Count = 1; //���� ���ø� �Ű����� ����
	scd.SampleDesc.Quality = 0; //�׷��� ����Ƽ

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //��ǥ�� �� �ڿ� ���� �ɼ� //���缳�� : [ǥ�� �Ǵ� ���ҽ��� ��� ������ ������� ����մϴ�.]
	scd.BufferCount = 1; //����ü���� ���� ��
	scd.OutputWindow = hwnd; //��� â�� ���� �ڵ�
	scd.Windowed = TRUE; //����� ������ ������� ����
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //���÷��� ȭ�鿡�� �ȼ��� ó���ϱ� ���� �ɼ�
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //����ü�� ���ۿ� ���� �ɼ�

	HRESULT hr;

	hr = D3D11CreateDeviceAndSwapChain(
		adapters[0].pAdapter, //IDXGI ���
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL, //����Ʈ���� ����̺� Ÿ��
		NULL, //��Ÿ�� ���̾ ���� �÷���
		NULL, //��� ���� �迭
		0, //�迭�� ��� ����
		D3D11_SDK_VERSION,//SDK ����
		&scd, //����ü�� ����
		this->swapchain.GetAddressOf(),
		this->device.GetAddressOf(),
		NULL, //�����Ǵ� ��� ����
		this->deviceConstext.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "����ü�� ������ �����߽��ϴ�.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D>	backBuffer;

	hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "���۰������� ����.");
		return false;
	}
	//���ҽ��� ������ �ϱ� ���� ����Ÿ�ٺ�

	hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->rederTargetView.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "����Ÿ�ٺ� ���� ����.");
		return false;
	}

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = this->windowWidth;
	depthStencilDesc.Height = this->windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "���� ���ٽ� ���� ������ �����߽��ϴ�.");
		return false;
	}

	hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "���� ���ٽ� �並 ������ �����߽��ϴ�.");
		return false;
	}
	this->deviceConstext->OMSetRenderTargets(1, this->rederTargetView.GetAddressOf(), this->depthStencilView.Get());

	//���� ���ٽ� ���� ����
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "���� ���ٽ� ���� ������ �����߽��ϴ�.");
		return false;
	}

	//����Ʈ ����
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = this->windowWidth;
	viewport.Height = this->windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//����Ʈ ����
	this->deviceConstext->RSSetViewports(1, &viewport);

	//������ ������ ����
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID; //ä�����
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;//3d �Ҷ� �տ����� ���̰� �ڿ����� �Ⱥ��̰� �ϴ� �̹��� ó���� �̰���
	hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "������ ���̴� ������Ʈ ������ �����߽��ϴ�.");
		return false;
	}

	spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceConstext.Get());
	spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Data\\fonts\\myfile.spritefont");

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "���� ������Ʈ ������ �����߽��ϴ�.");
		return false;
	}
	return true; 
}

bool Graphics::InitializeShaders()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements = ARRAYSIZE(layout);


	if (!vertexshader.Initialize(this->device,L"x64\\Debug\\vertexshader.cso",layout ,numElements))
		return false;

	if (!pixelshader.Initialize(this->device, L"x64\\Debug\\pixelshader.cso"))
		return false;

	return true;
}

bool Graphics::InitializeScene()
{	 //ȭ�� �߾��� 0��
	//�ؽ���
	Vertex v[] =
	{
		Vertex(-0.5f, -0.5f, 0.0f, 0.0f, 1.0f), //Bottom Left [0]
		Vertex(-0.5f,  0.5f, 0.0f, 0.0f, 0.0f), //Top LEFT    [1]
		Vertex( 0.5f,  0.5f, 0.0f, 1.0f, 0.0f), //Top Right   [2]
		Vertex( 0.5f, -0.5f, 0.0f, 1.0f, 1.0f), //Bottom Right[3]

	};

	HRESULT hr = this->vertexBuffer.Initialize(this->device.Get(), v, ARRAYSIZE(v));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "�������۸� �����ϴµ� �����߽��ϴ�.");
		return false;
	}

	DWORD indices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	//�ε�������

	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = indices;
	hr = this->indicesBuffer.Initialize(this->device.Get(), indices, ARRAYSIZE(indices));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "�ε��� ���� ������ �����߽��ϴ�");
		return hr;
	}

	hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\char_idle.png", nullptr, myTexture.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "WIC �ؽ��� ������ �����߽��ϴ�.");
		return false;
	}

	//��� ���� �ʱ�ȭ
	
	hr = this->constantBuffer.Initialize(this->device.Get(), this->deviceConstext.Get());
	if (FAILED(hr))
	{
		ErrorLogger::Log("��� ���� �ʱ�ȭ�� �����߽��ϴ�.");
		return false;
	}
	
	camera.SetPosition(0.0f, 0.0f, -2.0f);
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 1000.0f);

	return true;
}

