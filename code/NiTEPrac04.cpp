#include <iostream>

#include <NiTE.h>
#include <opencv2/opencv.hpp>

class NiteApp
{
public:

	// 初期化
	void initialize()
	{
		// UderTrackerを作成する
		userTracker.create();
	}

	// フレーム更新処理
	void update()
	{
		nite::UserTrackerFrameRef userFrame;
		userTracker.readFrame(&userFrame);
		depthImage = showUser(userFrame);
		cv::imshow("User", depthImage);
	}

private:

	// ユーザの検出
	cv::Mat showUser(nite::UserTrackerFrameRef& userFrame)
	{
		// ユーザに色を付ける
		static const cv::Scalar colors[] = {
			cv::Scalar(1, 0, 0),
			cv::Scalar(0, 1, 0),
			cv::Scalar(0, 0, 1),
			cv::Scalar(1, 1, 0),
			cv::Scalar(1, 0, 1),
			cv::Scalar(0, 1, 1),
			cv::Scalar(0.5, 0, 0),
			cv::Scalar(0, 0, 0.5),
			cv::Scalar(0.5, 0.5, 0),
		};

		cv::Mat depthImage;

		// depth フレームを取得する
		openni::VideoFrameRef depthFrame = userFrame.getDepthFrame();
		if (depthFrame.isValid()) {
			depthImage = cv::Mat(depthFrame.getHeight(), depthFrame.getWidth(), CV_8UC4);

			// Depth データおよびユーザインデックスを取得する
			openni::DepthPixel* depth = (openni::DepthPixel*)depthFrame.getData();
			const nite::UserId* pLavels = userFrame.getUserMap().getPixels();

			//// 1ピクセルずつ調べる
			for (int i = 0; i < (depthFrame.getDataSize() / sizeof(openni::DepthPixel)); ++i) {
				// カラー画像のインデックスを生成
				int index = i * 4;
				// 距離データを画像化する
				uchar* data = &depthImage.data[index];
				if (pLavels[i] != 0) {
					// 人を検出したピクセルにはユーザ番号で色を付ける
					data[0] *= colors[pLavels[i]][0];
					data[1] *= colors[pLavels[i]][1];
					data[2] *= colors[pLavels[i]][2];
				}
				else {
					// 人を検出しなかったピクセルには depth データを書き込む
					// 0-255のグレーデータを作成する
					// disatance : 10000 = gray : 255
					int gray = ~((depth[i] * 255) / 10000);
					data[0] = gray;
					data[1] = gray;
					data[2] = gray;
				}
			}
		}

		return depthImage;
	}

	nite::UserTracker userTracker;	// ユーザ検出
	cv::Mat depthImage;				// 可視化した Depth データ
};

int main(int argc, const char* argv[]) 
{
	try {
		// NiTEを初期化する
		nite::NiTE::initialize();

		// アプリケーションを初期化する
		NiteApp app;
		app.initialize();

		// メインループ
		while (1) {
			app.update();

			int key = cv::waitKey(10);
			if (key == 'q') {
				break;
			}
		}
	}
	catch (std::exception&) {
		std::cout << openni::OpenNI::getExtendedError() << std::endl;
	}

	return 0;
}