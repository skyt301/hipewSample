// hipewSample.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <vector>
#include <contrib\hipew\include\hipew.h>

int main()
{
	std::cout << "Hello HIP!\n";

	//hipewの初期化
	int driver, result;
	hipewInit(&driver, &result, HIPEW_INIT_HIPRTC | HIPEW_INIT_HIPDRIVER);

	//HIPの初期化
	hipError_t e = hipSuccess;
	e = hipInit(0);

	//デバイス総数の取得
	int countDevice;
	e = hipGetDeviceCount(&countDevice);

	std::cout << "Total device count :" << countDevice << std::endl;

	//HIPデバイスの取得
	hipDevice_t hipDevice;
	hipDeviceGet(&hipDevice, 0);
	char deviceName[64];
	e = hipDeviceGetName(deviceName, sizeof(deviceName), hipDevice);

	std::cout << "Device Name :" << deviceName << std::endl;

	//kernelの構築
	hipFunction_t function;
	{
		//埋め込みコード
		const char* code = R"(
			extern "C" {
				__device__ float sigmoid(float x)
				{
					return 1 / (1 + expf(x));
				}
				__global__ void testKernel(float *a)
				{
					a[threadIdx.x] = sigmoid((float)1 / a[threadIdx.x]);
				}
				
			}
		)";
		

		hiprtcProgram prog = nullptr;
		hiprtcResult re = HIPRTC_SUCCESS;

		const char* funcName = "testKernel";

		//hiprtcProgramの作成
		re = hiprtcCreateProgram(&prog, code, funcName, 0, 0, 0);
		std::vector<const char*> opts;
		opts.push_back("-I ../");

		//kernelのコンパイル
		re = hiprtcCompileProgram(prog, opts.size(), opts.data());
		if (re != HIPRTC_SUCCESS)
		{
			size_t logSize;
			//コンパイルログの出力
			hiprtcGetProgramLogSize(prog, &logSize);
			if (logSize)
			{
				std::string log(logSize, '\0');
				hiprtcGetProgramLog(prog, &log[0]);
				std::cout << log << '\n';
			};
		}
		size_t codeSize;
		re = hiprtcGetCodeSize(prog, &codeSize);

		std::vector<char> codec(codeSize);
		//コンパイル済みバイナリを抽出
		re = hiprtcGetCode(prog, codec.data());
		//hiprtcProgramの破棄
		re = hiprtcDestroyProgram(&prog);
		hipModule_t module;
		//コンパイル済みバイナリをデバイスにロード
		hipError_t ee = hipModuleLoadData(&module, codec.data());
		//実行可能デバイスカーネルを保持
		ee = hipModuleGetFunction(&function, module, funcName);


		float* managedFloat;
		//HIPManagedメモリを作成
		e = hipMallocManaged((hipDeviceptr_t*)&managedFloat, sizeof(float) * 4, hipMemAttachGlobal);

		//マネージドメモリを初期化
		managedFloat[0] = 1.0f;
		managedFloat[1] = 4.0f;
		managedFloat[2] = 8.0f;
		managedFloat[3] = 16.0f;

		std::cout << "before Computed.\n";
		for (int i = 0; i < 4; i++)
		{
			std::cout << "OriginalFloat[" << i << "]: " << managedFloat[i] << std::endl;
		}

		const void* args[] = { &managedFloat };

		//gridDimX,Y,Z=1, blockDimX, Y, Z=4, 1, 1(threadIdx.x軸で4つのスレッドで実行)
		//sharedMemBytes=0, hStream=NULL(デフォルトストリーム), kernelParams=args, extra=0
		e = hipModuleLaunchKernel(function, 1, 1, 1, 4, 1, 1, 0, NULL, (void**)args, 0);

		//デバイス上のすべてのストリームに記録されたコマンドを実行完了するのを待つ
		hipDeviceSynchronize();

		std::cout << "after Computed.\n";
		for (int i = 0; i < 4; i++)
		{
			std::cout << "managedFloat[" << i << "]: " << managedFloat[i] << std::endl;
		}

		//マネージドメモリの解放
		e = hipFree((hipDeviceptr_t)managedFloat);

	}
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
