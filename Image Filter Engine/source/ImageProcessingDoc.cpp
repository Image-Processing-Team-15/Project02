#include "stdafx.h"

#undef min
#undef max

#include "ImageProcessing.h"
#include "ImageProcessingDoc.h"
#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 피부색 판별 함수 (YCbCr 기반의 간단한 스킨 컬러 모델)
static bool IsSkinColor(BYTE r, BYTE g, BYTE b)
{
	// YCbCr 근사 변환 (정수 연산 사용)
	int y = (77 * r + 150 * g + 29 * b) >> 8;
	int cb = ((-43 * r - 85 * g + 128 * b) >> 8) + 128;
	int cr = ((128 * r - 107 * g - 21 * b) >> 8) + 128;

	// 일반적인 피부색 범위 (경험적 값, 필요시 조절 가능)
	if (cb >= 77 && cb <= 127 && cr >= 133 && cr <= 173)
		return true;
	return false;
}

IMPLEMENT_DYNCREATE(CImageProcessingDoc, CDocument)

BEGIN_MESSAGE_MAP(CImageProcessingDoc, CDocument)

	ON_COMMAND(ID_FILE_REVERT, &CImageProcessingDoc::OnFileRevert)

	ON_COMMAND(ID_STYLIZE_CARTOON, &CImageProcessingDoc::OnStylizeCartoon)
	ON_COMMAND(ID_STYLIZE_OILPAINTING, &CImageProcessingDoc::OnStylizeOilPainting)
	ON_COMMAND(ID_STYLIZE_POPART, &CImageProcessingDoc::OnStylizePopArt)
	ON_COMMAND(ID_STYLIZE_EMBOSS, &CImageProcessingDoc::OnStylizeEmboss)

	ON_COMMAND(ID_COLORMOOD_RETRO, &CImageProcessingDoc::OnColorMoodRetro)
	ON_COMMAND(ID_COLORMOOD_WARM, &CImageProcessingDoc::OnColorMoodWarm)
	ON_COMMAND(ID_COLORMOOD_COOL, &CImageProcessingDoc::OnColorMoodCool)
	ON_COMMAND(ID_COLORMOOD_BLOOM, &CImageProcessingDoc::OnColorMoodBloom)
	ON_COMMAND(ID_COLORMOOD_VIGNETTE, &CImageProcessingDoc::OnColorMoodVignette)

END_MESSAGE_MAP()


CImageProcessingDoc::CImageProcessingDoc()
{
	m_pImage = NULL;
}

CImageProcessingDoc::~CImageProcessingDoc()
{
	if (NULL != m_pImage)
		delete m_pImage;
}

BOOL CImageProcessingDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	m_pImage = new CxImage;
	m_pImage->Load(lpszPathName, FindType(lpszPathName));

	return TRUE;
}

BOOL CImageProcessingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}


void CImageProcessingDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}


#ifdef _DEBUG
void CImageProcessingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CImageProcessingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif 


CString CImageProcessingDoc::FindExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len - 1; i >= 0; i--) {
		if (name[i] == '.') {
			return name.Mid(i + 1);
		}
	}
	return CString(_T(""));
}

CString CImageProcessingDoc::RemoveExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len - 1; i >= 0; i--) {
		if (name[i] == '.') {
			return name.Mid(0, i);
		}
	}
	return name;
}

int CImageProcessingDoc::FindType(const CString& ext)
{
	return CxImage::GetTypeIdFromName(ext);
}

void CImageProcessingDoc::OnFileRevert()
{
	if (!m_pImage || GetPathName().IsEmpty())
	{
		AfxMessageBox(_T("Cannot revert. File path is unknown or no image is loaded."));
		return;
	}

	CString strPath = GetPathName();

	delete m_pImage;
	m_pImage = NULL;

	m_pImage = new CxImage;
	if (!m_pImage->Load(strPath, FindType(strPath)))
	{
		AfxMessageBox(_T("Error: Failed to reload the original image from disk."));
		delete m_pImage;
		m_pImage = NULL;
		return;
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::ApplyConvolution3x3(double kernel[3][3])
{
	if (!m_pImage) return;

	CxImage tempImage(*m_pImage);

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			double sumR = 0.0, sumG = 0.0, sumB = 0.0;

			for (int ky = 0; ky < 3; ky++)
			{
				for (int kx = 0; kx < 3; kx++)
				{
					RGBQUAD pixel = tempImage.GetPixelColor(x + kx - 1, y + ky - 1);

					double k = kernel[ky][kx];
					sumR += pixel.rgbRed * k;
					sumG += pixel.rgbGreen * k;
					sumB += pixel.rgbBlue * k;
				}
			}

			RGBQUAD newPixel;
			newPixel.rgbRed = Clamp(sumR);
			newPixel.rgbGreen = Clamp(sumG);
			newPixel.rgbBlue = Clamp(sumB);
			newPixel.rgbReserved = 0;

			m_pImage->SetPixelColor(x, y, newPixel);
		}
	}
}

BYTE CImageProcessingDoc::Clamp(double value)
{
	if (value > 255.0) return 255;
	if (value < 0.0) return 0;
	return (BYTE)value;
}

void CImageProcessingDoc::OnStylizeCartoon()
{
	// 1. 기본 체크
	if (!m_pImage) return;

	// 컬러 이미지(24bit 이상)만 처리
	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Cartoon 필터는 24bit 이상의 컬러 이미지에만 적용 가능합니다."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 원본 이미지 백업 (얼굴 마스크 / 에지 추출 등에 사용)
	CxImage original(*m_pImage);

	// 2. 피부(얼굴) 후보 마스크 생성
	std::vector<BYTE> skinMask(width * height, 0);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			RGBQUAD c = original.GetPixelColor(x, y);
			if (IsSkinColor(c.rgbRed, c.rgbGreen, c.rgbBlue))
			{
				skinMask[y * width + x] = 1; // 피부 후보
			}
		}
	}

	// 3. 전체 기본 노이즈 제거 (가우시안 블러 2회)
	//    - 사진 노이즈를 줄이고, 이후 색상 양자화를 위한 준비
	double gauss[3][3] =
	{
		{ 1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0 },
		{ 2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0 },
		{ 1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0 }
	};
	ApplyConvolution3x3(gauss);   // 1차 블러
	ApplyConvolution3x3(gauss);   // 2차 블러

	// 4. 피부 영역 미용 보정 
	//    - 피부 후보 영역에 더 강한 스무딩 + 밝고 균일한 톤 적용
	const int beautyRadius = 1;   // 3x3 로컬 평균 사용

	for (int y = 1; y < height - 1; ++y)
	{
		for (int x = 1; x < width - 1; ++x)
		{
			if (skinMask[y * width + x] == 0)
				continue;   // 피부가 아닌 영역은 패스

			int sumR = 0, sumG = 0, sumB = 0, cnt = 0;

			// 주변 3x3 평균 색 계산
			for (int j = -beautyRadius; j <= beautyRadius; ++j)
			{
				for (int i = -beautyRadius; i <= beautyRadius; ++i)
				{
					RGBQUAD nb = m_pImage->GetPixelColor(x + i, y + j);
					sumR += nb.rgbRed;
					sumG += nb.rgbGreen;
					sumB += nb.rgbBlue;
					++cnt;
				}
			}

			int avgR = sumR / cnt;
			int avgG = sumG / cnt;
			int avgB = sumB / cnt;

			// 밝고 균일한 아바타 피부 톤으로 변환
			int yv = (77 * avgR + 150 * avgG + 29 * avgB) >> 8; // 밝기
			int r = yv + 18;
			int g = yv + 10;
			int b = yv + 6;

			r = (int)(r * 1.02);
			g = (int)(g * 0.98);
			b = (int)(b * 0.96);

			RGBQUAD pix;
			// [수정됨] ClampToByte -> Clamp
			pix.rgbRed = Clamp(r);
			pix.rgbGreen = Clamp(g);
			pix.rgbBlue = Clamp(b);

			m_pImage->SetPixelColor(x, y, pix);
		}
	}

	// 5. 전체 색상 단순화 (Color Quantization)
	//    - 머리카락/옷/배경을 소수의 색 단계로 양자화 → 만화 같은 큰 색 덩어리
	const int   nLevels = 5;              // 단계 수 (4~6 사이에서 조절 가능)
	const double step = 256.0 / nLevels;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			RGBQUAD c = m_pImage->GetPixelColor(x, y);

			int binR = (int)(c.rgbRed / step);
			int binG = (int)(c.rgbGreen / step);
			int binB = (int)(c.rgbBlue / step);

			double newR = binR * step + step / 2.0;
			double newG = binG * step + step / 2.0;
			double newB = binB * step + step / 2.0;

			// 약간 따뜻한 톤으로 보정
			newR *= 1.02;
			newG *= 1.01;
			newB *= 0.99;

			// [수정됨] ClampToByte -> Clamp
			c.rgbRed = Clamp((int)newR);
			c.rgbGreen = Clamp((int)newG);
			c.rgbBlue = Clamp((int)newB);

			m_pImage->SetPixelColor(x, y, c);
		}
	}

	// 6. 전체 대비/채도 조정 (그림 느낌 강화) 
	const double contrast = 1.15;
	const double satScale = 1.10;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			RGBQUAD c = m_pImage->GetPixelColor(x, y);

			int r = (int)((c.rgbRed - 128) * contrast + 128);
			int g = (int)((c.rgbGreen - 128) * contrast + 128);
			int b = (int)((c.rgbBlue - 128) * contrast + 128);

			int gray = (r + g + b) / 3;
			r = gray + (int)((r - gray) * satScale);
			g = gray + (int)((g - gray) * satScale);
			b = gray + (int)((b - gray) * satScale);

			// [수정됨] ClampToByte -> Clamp
			c.rgbRed = Clamp(r);
			c.rgbGreen = Clamp(g);
			c.rgbBlue = Clamp(b);

			m_pImage->SetPixelColor(x, y, c);
		}
	}

	// 7. 에지 이미지 생성 (검은 만화 선용)
	//    - 원본 사진에서 강한 윤곽선을 추출해서, 결과 이미지 위에 검은 선으로 덧그리기
	CxImage edgeImg(original);
	edgeImg.GrayScale();   // R=G=B인 그레이 이미지

	// 컨볼루션 재사용을 위해 잠시 m_pImage 를 edgeImg 로 변경
	CxImage* pBackup = m_pImage;
	m_pImage = &edgeImg;

	double edgeKernel[3][3] =
	{
		{ -1.0, -1.0, -1.0 },
		{ -1.0,  8.0, -1.0 },
		{ -1.0, -1.0, -1.0 }
	};
	ApplyConvolution3x3(edgeKernel);   // 에지 강화

	// 다시 원래 이미지 포인터로 복원
	m_pImage = pBackup;


	// 8. 에지를 이용한 검은 붓펜 아웃라인 오버레이
	//    - 1단계: edgeThreshold 로 1차 후보 필터링
	//    - 2단계: 3x3 이웃 중 에지가 충분히 많은 픽셀만 진짜 선으로 사용
	const int edgeThreshold = 110; // 값이 높을수록 선이 적고 깔끔해짐 (100~130 사이 튜닝)
	const int minNeighborEdges = 3;   // 이웃 에지 픽셀 최소 개수 (노이즈 제거용)

	for (int y = 1; y < height - 1; ++y)
	{
		for (int x = 1; x < width - 1; ++x)
		{
			RGBQUAD e = edgeImg.GetPixelColor(x, y);
			int edgeVal = e.rgbRed;  // 그레이스케일이므로 R=G=B 가정

			// 1차: 에지 후보인지 확인
			if (edgeVal <= edgeThreshold)
				continue;

			// 2차: 주변 3x3 이웃 중에서 에지 픽셀이 얼마나 있는지 카운트
			int neighborCount = 0;
			for (int j = -1; j <= 1; ++j)
			{
				for (int i = -1; i <= 1; ++i)
				{
					if (i == 0 && j == 0) continue; // 자기 자신 제외

					RGBQUAD enb = edgeImg.GetPixelColor(x + i, y + j);
					int valNb = enb.rgbRed;

					if (valNb > edgeThreshold)
						++neighborCount;
				}
			}

			// 이웃 에지가 너무 적으면 고립된 점(노이즈)으로 판단하고 무시
			if (neighborCount < minNeighborEdges)
				continue;

			// 여기까지 통과한 픽셀만 "진짜 만화 선"으로 사용
			RGBQUAD c = m_pImage->GetPixelColor(x, y);
			c.rgbRed = 0;
			c.rgbGreen = 0;
			c.rgbBlue = 0;
			m_pImage->SetPixelColor(x, y, c);
		}
	}

	// 9. 화면 갱신 
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnStylizeOilPainting()
{
	// 1. 이미지가 열려있는지 확인
	if (!m_pImage) return;

	// 컬러 이미지가 아니면 처리 불가
	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Oil Painting filter can only be applied to color images."));
		return;
	}

	// --- 필터 튜닝 값 (상수) ---
	const int radius = 4;        // 붓 터치 크기 (클수록 뭉개짐이 커짐, 2~5 추천)
	const int intensityLevels = 20; // 밝기 단계 (작을수록 색이 단순해짐, 10~30 추천)
	// --------------------------

	// 2. 원본 이미지를 복사 (계산 중 원본 데이터 참조용)
	CxImage tempImage(*m_pImage);

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 3. 전체 픽셀 순회
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 각 밝기 레벨별로 카운트할 배열과 색상 합계 배열 초기화
			// intensityLevels 개수만큼 0으로 초기화
			int intensityCount[256] = { 0 };
			int sumR[256] = { 0 };
			int sumG[256] = { 0 };
			int sumB[256] = { 0 };

			// 4. 커널(주변 픽셀) 순회 (-radius ~ +radius)
			for (int ky = -radius; ky <= radius; ky++)
			{
				for (int kx = -radius; kx <= radius; kx++)
				{
					int nx = x + kx;
					int ny = y + ky;

					// 이미지 범위를 벗어나지 않도록 체크
					if (nx >= 0 && nx < width && ny >= 0 && ny < height)
					{
						RGBQUAD pixel = tempImage.GetPixelColor(nx, ny);

						// 현재 픽셀의 밝기(Intensity) 계산 (0~255)
						int currentIntensity = (pixel.rgbRed + pixel.rgbGreen + pixel.rgbBlue) / 3;

						// 밝기를 intensityLevels 단계로 양자화(Quantization)
						// 예: 255를 20단계로 나누면, 밝기 0~12는 0번 방, 13~25는 1번 방...
						int binIndex = (currentIntensity * intensityLevels) / 255;

						// 인덱스 안전 장치
						if (binIndex >= intensityLevels) binIndex = intensityLevels - 1;

						// 해당 밝기 단계에 카운트 증가 및 색상 누적
						intensityCount[binIndex]++;
						sumR[binIndex] += pixel.rgbRed;
						sumG[binIndex] += pixel.rgbGreen;
						sumB[binIndex] += pixel.rgbBlue;
					}
				}
			}

			// 5. 가장 빈도수가 높은(픽셀이 제일 많은) 밝기 단계 찾기
			int maxCount = -1;
			int maxIndex = 0;

			for (int i = 0; i < intensityLevels; i++)
			{
				if (intensityCount[i] > maxCount)
				{
					maxCount = intensityCount[i];
					maxIndex = i;
				}
			}

			// 6. 가장 많이 나온 단계의 '평균 색상' 계산
			RGBQUAD newPixel;
			if (maxCount > 0) // 0으로 나누기 방지
			{
				newPixel.rgbRed = (BYTE)(sumR[maxIndex] / maxCount);
				newPixel.rgbGreen = (BYTE)(sumG[maxIndex] / maxCount);
				newPixel.rgbBlue = (BYTE)(sumB[maxIndex] / maxCount);
				newPixel.rgbReserved = 0;
			}
			else
			{
				// 예외 상황: 원래 색 유지
				newPixel = tempImage.GetPixelColor(x, y);
			}

			// 7. 결과 이미지에 적용
			m_pImage->SetPixelColor(x, y, newPixel);
		}
	}

	// 8. 화면 갱신
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnStylizePopArt()
{
	const int nLevels = 4;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Pop Art filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	const double stepSize = 256.0 / nLevels;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			int binIndex_R = (int)(color.rgbRed / stepSize);
			double newRed = (binIndex_R * stepSize) + (stepSize / 2.0);

			int binIndex_G = (int)(color.rgbGreen / stepSize);
			double newGreen = (binIndex_G * stepSize) + (stepSize / 2.0);

			int binIndex_B = (int)(color.rgbBlue / stepSize);
			double newBlue = (binIndex_B * stepSize) + (stepSize / 2.0);


			color.rgbRed = (BYTE)std::min(255.0, std::max(0.0, newRed));
			color.rgbGreen = (BYTE)std::min(255.0, std::max(0.0, newGreen));
			color.rgbBlue = (BYTE)std::min(255.0, std::max(0.0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnStylizeEmboss()
{
	if (!m_pImage) return;

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	CxImage I_Gray(*m_pImage);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD color = I_Gray.GetPixelColor(x, y);

			int grayValue = static_cast<int>(0.299 * color.rgbRed + 0.587 * color.rgbGreen + 0.114 * color.rgbBlue);

			color.rgbRed = color.rgbGreen = color.rgbBlue = Clamp(grayValue);

			I_Gray.SetPixelColor(x, y, color);
		}
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD gray_pixel = I_Gray.GetPixelColor(x, y);
			m_pImage->SetPixelColor(x, y, gray_pixel);
		}
	}

	double kernel_emboss[3][3] = {
		{-2.0, -1.0, 0.0},
		{-1.0, 1.0, 1.0},
		{ 0.0, 1.0, 2.0}
	};

	ApplyConvolution3x3(kernel_emboss);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			double embossed_value = color.rgbRed;

			int final_gray_value = Clamp(embossed_value);

			RGBQUAD final_pixel;
			final_pixel.rgbRed = final_gray_value;
			final_pixel.rgbGreen = final_gray_value;
			final_pixel.rgbBlue = final_gray_value;
			final_pixel.rgbReserved = 0;

			m_pImage->SetPixelColor(x, y, final_pixel);
		}
	}

	UpdateAllViews(NULL);
}


void CImageProcessingDoc::OnColorMoodBloom()
{
	if (!m_pImage) return;

	const float strength = 0.7f;
	const int threshold = 0;

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	CxImage original_copy(*m_pImage);

	CxImage I_Bright(*m_pImage);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD color = I_Bright.GetPixelColor(x, y);
			int L = (color.rgbRed + color.rgbGreen + color.rgbBlue) / 3;

			if (L < threshold) {
				color.rgbRed = color.rgbGreen = color.rgbBlue = 0;
				I_Bright.SetPixelColor(x, y, color);
			}
		}
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD bright_pixel = I_Bright.GetPixelColor(x, y);
			m_pImage->SetPixelColor(x, y, bright_pixel);
		}
	}

	double kernel_gaussian[3][3] = {
		{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
		{2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0},
		{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}
	};

	ApplyConvolution3x3(kernel_gaussian);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD blur_color = m_pImage->GetPixelColor(x, y);
			RGBQUAD original_color = original_copy.GetPixelColor(x, y);

			RGBQUAD final_color;
			final_color.rgbRed = Clamp(original_color.rgbRed + (int)(blur_color.rgbRed * strength));
			final_color.rgbGreen = Clamp(original_color.rgbGreen + (int)(blur_color.rgbGreen * strength));
			final_color.rgbBlue = Clamp(original_color.rgbBlue + (int)(blur_color.rgbBlue * strength));
			final_color.rgbReserved = 0;

			m_pImage->SetPixelColor(x, y, final_color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodRetro()
{
	const double SEP_R_R = 0.393; const double SEP_R_G = 0.769; const double SEP_R_B = 0.189;
	const double SEP_G_R = 0.349; const double SEP_G_G = 0.686; const double SEP_G_B = 0.168;
	const double SEP_B_R = 0.272; const double SEP_B_G = 0.534; const double SEP_B_B = 0.131;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Retro filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			double originalRed = color.rgbRed;
			double originalGreen = color.rgbGreen;
			double originalBlue = color.rgbBlue;

			double newRed = (originalRed * SEP_R_R) + (originalGreen * SEP_R_G) + (originalBlue * SEP_R_B);
			double newGreen = (originalRed * SEP_G_R) + (originalGreen * SEP_G_G) + (originalBlue * SEP_B_B);
			double newBlue = (originalRed * SEP_B_R) + (originalGreen * SEP_B_G) + (originalBlue * SEP_B_B);

			color.rgbRed = (BYTE)std::min(255.0, std::max(0.0, newRed));
			color.rgbGreen = (BYTE)std::min(255.0, std::max(0.0, newGreen));
			color.rgbBlue = (BYTE)std::min(255.0, std::max(0.0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodWarm()
{
	const int nRedAdjust = 25;
	const int nGreenAdjust = 10;
	const int nBlueAdjust = -15;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Warm filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			int newRed = color.rgbRed + nRedAdjust;
			int newGreen = color.rgbGreen + nGreenAdjust;
			int newBlue = color.rgbBlue + nBlueAdjust;

			color.rgbRed = (BYTE)std::min(255, std::max(0, newRed));
			color.rgbGreen = (BYTE)std::min(255, std::max(0, newGreen));
			color.rgbBlue = (BYTE)std::min(255, std::max(0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodCool()
{
	const int nRedAdjust = -15;
	const int nGreenAdjust = -10;
	const int nBlueAdjust = 25;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Cool filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			int newRed = color.rgbRed + nRedAdjust;
			int newGreen = color.rgbGreen + nGreenAdjust;
			int newBlue = color.rgbBlue + nBlueAdjust;

			color.rgbRed = (BYTE)std::min(255, std::max(0, newRed));
			color.rgbGreen = (BYTE)std::min(255, std::max(0, newGreen));
			color.rgbBlue = (BYTE)std::min(255, std::max(0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodVignette()
{
	const double VIGNETTE_STRENGTH = 1.3;

	if (!m_pImage) return;

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	double centerX = width / 2.0;
	double centerY = height / 2.0;

	double maxDistance = sqrt((centerX * centerX) + (centerY * centerY));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			double currentDistance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));

			double ratio = (currentDistance / maxDistance) * VIGNETTE_STRENGTH;

			double brightnessFactor = 1.0 - ratio;

			brightnessFactor = std::min(1.0, std::max(0.0, brightnessFactor));

			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			color.rgbRed = (BYTE)(color.rgbRed * brightnessFactor);
			color.rgbGreen = (BYTE)(color.rgbGreen * brightnessFactor);
			color.rgbBlue = (BYTE)(color.rgbBlue * brightnessFactor);

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}
