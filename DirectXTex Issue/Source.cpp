#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXTex/DirectXTex.h>

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

HWND CreateWindowReturnHandle();
HRESULT InitializeD3D(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext, ComPtr<IDXGISwapChain>& swapChain, HWND hwnd);

int WINAPI wWinMain(HINSTANCE hInstance_, HINSTANCE hPrevInstance_, PWSTR pCmdLine_, int nCmdShow_)
{
	HRESULT hr = CoInitialize(NULL); //We need to call coinitialize for DirectXTex
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "CoInitialize failed.", "Error", 0);
		return -1;
	}

	HWND hwnd = CreateWindowReturnHandle();
	if (hwnd == NULL)
	{
		MessageBoxA(NULL, "Failed to create window", "Error",  0);
		return -1;
	}

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

	hr = InitializeD3D(device, deviceContext, swapChain, hwnd);

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Failed to initialize d3d11.", "Error",  0);
		return -1;
	}

	DirectX::ScratchImage scratch;
	hr = DirectX::LoadFromTGAFile(L"test.tga", nullptr, scratch);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Failed to load tga file with DirectXTex.", "Error", 0);
		return -1;
	}

	const DirectX::Image* image = scratch.GetImage(0, 0, 0);
	if (image == nullptr)
	{
		MessageBoxA(NULL, "Image ptr was null from scratch.", "Error", 0);
		return -1;
	}

	CD3D11_TEXTURE2D_DESC textureDesc(image->format, image->width, image->height);
	ID3D11Texture2D* p2DTexture = nullptr;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = image->pixels;
	initialData.SysMemPitch = (UINT)image->rowPitch;
	hr = device->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "CreateTexture2D failed.", "Error",  0);
		return -1;
	}

	return 0;
}

HWND CreateWindowReturnHandle()
{
	WNDCLASSEXA wc = {}; //Our Window Class (This has to be filled before our window can be created) See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633577(v=vs.85).aspx
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //Flags [Redraw on width/height change from resize/movement] See: https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
	wc.lpfnWndProc = DefWindowProcA; //Pointer to Window Proc function for handling messages from this window
	wc.cbClsExtra = 0; //# of extra bytes to allocate following the window-class structure. We are not currently using this.
	wc.cbWndExtra = 0; //# of extra bytes to allocate following the window instance. We are not currently using this.
	wc.hInstance = GetModuleHandle(NULL); //Handle to the instance that contains the Window Procedure
	wc.hIcon = NULL;   //Handle to the class icon. Must be a handle to an icon resource. We are not currently assigning an icon, so this is null.
	wc.hIconSm = NULL; //Handle to small icon for this class. We are not currently assigning an icon, so this is null.
	wc.hCursor = NULL; //Default Cursor - If we leave this null, we have to explicitly set the cursor's shape each time it enters the window.
	wc.hbrBackground = NULL; //Handle to the class background brush for the window's background color - we will leave this blank for now and later set this to black. For stock brushes, see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd144925(v=vs.85).aspx
	wc.lpszMenuName = NULL; //Pointer to a null terminated character string for the menu. We are not using a menu yet, so this will be NULL.
	wc.lpszClassName = "winclass"; //Pointer to null terminated string of our class name for this window.
	wc.cbSize = sizeof(WNDCLASSEXA); //Need to fill in the size of our struct for cbSize
	RegisterClassExA(&wc); // Register the class so that it is usable.

	HWND hwnd = CreateWindowExA(0, //Extended Windows style - we are using the default. For other options, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ff700543(v=vs.85).aspx
		"winclass", //Window class name
		"title", //Window Title
		0, //Windows style - See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
		0, //Window X Position
		0, //Window Y Position
		800, //Window Width
		600, //Window Height
		NULL, //Handle to parent of this window. Since this is the first window, it has no parent window.
		NULL, //Handle to menu or child window identifier. Can be set to NULL and use menu in WindowClassEx if a menu is desired to be used.
		GetModuleHandle(NULL), //Handle to the instance of module to be used with this window
		nullptr); //Parameter passed to create window 'WM_NCCREATE'
	return hwnd;
}

HRESULT InitializeD3D(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext, ComPtr<IDXGISwapChain>& swapChain, HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC scd = { 0 };

	scd.BufferDesc.Width = 800;
	scd.BufferDesc.Height = 600;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1; //double buffered by default in windowed mode
	scd.OutputWindow = hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, //IDXGI Adapter
												D3D_DRIVER_TYPE_HARDWARE,
												0, //FOR SOFTWARE DRIVER TYPE
												D3D11_CREATE_DEVICE_DEBUG, //FLAGS FOR RUNTIME LAYERS
												nullptr, //FEATURE LEVELS ARRAY
												0, //# OF FEATURE LEVELS IN ARRAY
												D3D11_SDK_VERSION,
												&scd, //Swapchain description
												&swapChain, //Swapchain Address
												&device, //Device Address
												NULL, //Supported feature level
												&deviceContext); //Device Context Address

	return hr;
}