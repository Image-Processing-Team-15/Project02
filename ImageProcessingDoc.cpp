
// ImageProcessingDoc.cpp : Implement a CImageProcessingDoc Class
//

#include "stdafx.h"

#undef min
#undef max

#include "ImageProcessing.h"
#include "ImageProcessingDoc.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImageProcessingDoc

IMPLEMENT_DYNCREATE(CImageProcessingDoc, CDocument)

BEGIN_MESSAGE_MAP(CImageProcessingDoc, CDocument)

	// File Menu
	ON_COMMAND(ID_FILE_REVERT, &CImageProcessingDoc::OnFileRevert)

	// Stylization Filters
	ON_COMMAND(ID_STYLIZE_CARTOON, &CImageProcessingDoc::OnStylizeCartoon)
	ON_COMMAND(ID_STYLIZE_OILPAINTING, &CImageProcessingDoc::OnStylizeOilPainting)
	ON_COMMAND(ID_STYLIZE_POPART, &CImageProcessingDoc::OnStylizePopArt)
	ON_COMMAND(ID_STYLIZE_EMBOSS, &CImageProcessingDoc::OnStylizeEmboss)

	// Color & Mood Filters
	ON_COMMAND(ID_COLORMOOD_RETRO, &CImageProcessingDoc::OnColorMoodRetro)
	ON_COMMAND(ID_COLORMOOD_WARM, &CImageProcessingDoc::OnColorMoodWarm) 
	ON_COMMAND(ID_COLORMOOD_COOL, &CImageProcessingDoc::OnColorMoodCool) 
	ON_COMMAND(ID_COLORMOOD_BLOOM, &CImageProcessingDoc::OnColorMoodBloom)
	ON_COMMAND(ID_COLORMOOD_VIGNETTE, &CImageProcessingDoc::OnColorMoodVignette)

END_MESSAGE_MAP()


// CImageProcessingDoc Contruction/Destuction

CImageProcessingDoc::CImageProcessingDoc()
{
	//// TODO: Add an one-time generating code here
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

	// TODO: load imagefile // DONE
	m_pImage = new CxImage;
	m_pImage->Load(lpszPathName, FindType(lpszPathName));

	return TRUE;
}

BOOL CImageProcessingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	//// TODO: Add a re-initialization code here
	//// SDI documents will reuse this article

	return TRUE;
}




// CImageProcessingDoc serialization

void CImageProcessingDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		//// TODO: Add a saving code here
	}
	else
	{
		//// TODO: Add a loading code here
	}
}


// CImageProcessingDoc diagnosis

#ifdef _DEBUG
void CImageProcessingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CImageProcessingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CImageProcessingDoc command

CString CImageProcessingDoc::FindExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len-1; i >= 0; i--){
		if (name[i] == '.'){
			return name.Mid(i+1);
		}
	}
	return CString(_T(""));
}

CString CImageProcessingDoc::RemoveExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len-1; i >= 0; i--){
		if (name[i] == '.'){
			return name.Mid(0,i);
		}
	}
	return name;
}

int CImageProcessingDoc::FindType(const CString& ext)
{
	return CxImage::GetTypeIdFromName(ext);
}

// 파일 다시 불러오기
void CImageProcessingDoc::OnFileRevert()
{
	// 1. 이미지가 열려있는지, 파일 경로가 있는지 확인합니다.
	if (!m_pImage || GetPathName().IsEmpty())
	{
		AfxMessageBox(_T("Cannot revert. File path is unknown or no image is loaded."));
		return;
	}

	// 2. 현재 문서에 저장된 파일 경로를 가져옵니다.
	CString strPath = GetPathName();

	// 3. 기존에 수정되었던 m_pImage 객체를 메모리에서 삭제합니다.
	delete m_pImage;
	m_pImage = NULL;

	// 4. 새 CxImage 객체를 만들고, 저장된 경로(strPath)를 이용해
	//    디스크에서 원본 파일을 다시 로드합니다.
	m_pImage = new CxImage;
	if (!m_pImage->Load(strPath, FindType(strPath)))
	{
		// 만약 원본 파일이 삭제되었거나 하면 로드에 실패할 수 있습니다.
		AfxMessageBox(_T("Error: Failed to reload the original image from disk."));
		delete m_pImage;
		m_pImage = NULL;
		return;
	}

	// 5. 모든 뷰(View)에 화면을 새로 고침하라고 알립니다.
	UpdateAllViews(NULL);
}

// ==========================================================
//  2D Convolution Engine (3x3)
//  3x3 커널 배열을 입력받아 이미지 전체에 합성곱 수행
// ==========================================================
void CImageProcessingDoc::ApplyConvolution3x3(double kernel[3][3])
{
	if (!m_pImage) return;

	// 1. 원본 이미지를 복사합니다. (계산 중에는 원본 값이 필요하므로)
	CxImage tempImage(*m_pImage);

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 2. 이미지 순회 (가장자리 1픽셀은 계산에서 제외: 1 ~ size-1)
	//    (가장자리는 이웃 픽셀이 없어서 계산이 복잡해지므로 보통 건너뜁니다)
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			double sumR = 0.0, sumG = 0.0, sumB = 0.0;

			// 3. 3x3 커널 적용 (Convolution 핵심 로직)
			for (int ky = 0; ky < 3; ky++)
			{
				for (int kx = 0; kx < 3; kx++)
				{
					// 주변 픽셀(neighbor) 가져오기
					// (x-1, y-1) 부터 (x+1, y+1) 까지
					RGBQUAD pixel = tempImage.GetPixelColor(x + kx - 1, y + ky - 1);

					// 커널 값과 곱해서 누적
					double k = kernel[ky][kx];
					sumR += pixel.rgbRed * k;
					sumG += pixel.rgbGreen * k;
					sumB += pixel.rgbBlue * k;
				}
			}

			// 4. 결과값 클램핑 및 저장
			RGBQUAD newPixel;
			newPixel.rgbRed = Clamp(sumR);
			newPixel.rgbGreen = Clamp(sumG);
			newPixel.rgbBlue = Clamp(sumB);
			newPixel.rgbReserved = 0;

			m_pImage->SetPixelColor(x, y, newPixel);
		}
	}
}

// [도우미 함수] 값의 범위를 0~255로 제한
BYTE CImageProcessingDoc::Clamp(double value)
{
	if (value > 255.0) return 255;
	if (value < 0.0)   return 0;
	return (BYTE)value;
}

// ----------------------
// Stylization Filters
// ----------------------
void CImageProcessingDoc::OnStylizeCartoon()
{
	// TODO: 카툰 필터 구현 코드 추가
	if (!m_pImage) return;
	AfxMessageBox(_T("Cartoon Filter 실행"));
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
	// --- 필터 튜닝 값 (상수) ---
	// "Posterization" 레벨입니다.
	// 0~255의 색상 범위를 몇 개의 "계단"으로 단순화할지 결정합니다.
	// 값이 낮을수록(예: 3 또는 4) 색상이 더 단순해지고 "Pop Art" 느낌이 강해집니다.
	const int nLevels = 4;
	// --------------------------

	// 1. 이미지가 열려있는지 확인합니다.
	if (!m_pImage) return;

	// 2. 컬러 이미지가 아니면(그레이스케일이면) 필터를 적용할 수 없습니다.
	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Pop Art filter can only be applied to color images."));
		return;
	}

	// 3. 이미지의 가로, 세로 크기를 가져옵니다.
	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 4. Posterization 로직에 필요한 "계단(bin)의 크기"를 계산합니다.
	// 256개의 값을 nLevels 단계로 나눕니다. (예: 256 / 5 = 51.2)
	const double stepSize = 256.0 / nLevels;

	// 5. 모든 픽셀을 순회하기 위한 2중 반복문
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 6. (x, y) 위치의 픽셀 색상 값(RGB)을 읽어옵니다.
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			// 7. Pop Art (Posterization) 로직 적용
			// R, G, B 각 채널에 대해 독립적으로 수행합니다.

			// --- Red 채널 ---
			// 7a. 현재 R값이 몇 번째 "계단"에 속하는지 계산합니다.
			// (예: R=70, stepSize=51.2 -> 70 / 51.2 = 1.36... -> (int)1 -> 1번째 계단)
			int binIndex_R = (int)(color.rgbRed / stepSize);
			// 7b. 그 계단의 "대표값"(중간값)으로 색상을 변경합니다.
			// (예: 1 * 51.2 + (51.2 / 2) = 76.8)
			double newRed = (binIndex_R * stepSize) + (stepSize / 2.0);

			// --- Green 채널 ---
			int binIndex_G = (int)(color.rgbGreen / stepSize);
			double newGreen = (binIndex_G * stepSize) + (stepSize / 2.0);

			// --- Blue 채널 ---
			int binIndex_B = (int)(color.rgbBlue / stepSize);
			double newBlue = (binIndex_B * stepSize) + (stepSize / 2.0);


			// 8. 클램핑(Clamping)
			// (이 로직은 0~255 범위를 벗어나지 않지만, BYTE 변환을 위해 명시적으로 둡니다.)
			color.rgbRed = (BYTE)std::min(255.0, std::max(0.0, newRed));
			color.rgbGreen = (BYTE)std::min(255.0, std::max(0.0, newGreen));
			color.rgbBlue = (BYTE)std::min(255.0, std::max(0.0, newBlue));

			// 9. 계산된 새 색상 값으로 (x, y) 위치의 픽셀을 덮어씁니다.
			m_pImage->SetPixelColor(x, y, color);
		}
	}

	// 10. 모든 뷰(View)에 화면을 새로 고침하라고 알립니다.
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnStylizeEmboss()
{
	// TODO: 엠보스 필터 구현 코드 추가
	if (!m_pImage) return;
	AfxMessageBox(_T("Emboss Filter 실행"));
	UpdateAllViews(NULL);
}


// ----------------------
// Color & Mood Filters
// ----------------------
void CImageProcessingDoc::OnColorMoodRetro()
{
	// --- 필터 튜닝 값 (상수) ---
	// "세피아(Sepia)" 톤 변환에는 표준적으로 사용되는 3x3 행렬(Matrix) 값이 있습니다.
	// 이 값들은 각 픽셀의 (R, G, B) 값을 조합하여 새로운 (R, G, B) 값을 만듭니다.
	// (예: newRed = R*0.393 + G*0.769 + B*0.189)
	const double SEP_R_R = 0.393; const double SEP_R_G = 0.769; const double SEP_R_B = 0.189;
	const double SEP_G_R = 0.349; const double SEP_G_G = 0.686; const double SEP_G_B = 0.168;
	const double SEP_B_R = 0.272; const double SEP_B_G = 0.534; const double SEP_B_B = 0.131;
	// --------------------------

	// 1. 이미지가 열려있는지 확인합니다.
	if (!m_pImage) return;

	// 2. 컬러 이미지가 아니면(그레이스케일이면) 필터를 적용할 수 없습니다.
	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Retro filter can only be applied to color images."));
		return;
	}

	// 3. 이미지의 가로, 세로 크기를 가져옵니다.
	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 4. 모든 픽셀을 순회하기 위한 2중 반복문
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 5. (x, y) 위치의 픽셀 색상 값(RGB)을 읽어옵니다.
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			// 원본 R, G, B 값을 임시 변수에 저장 (계산에 필요)
			double originalRed = color.rgbRed;
			double originalGreen = color.rgbGreen;
			double originalBlue = color.rgbBlue;

			// 6. Retro (Sepia) 로직 적용 (위의 행렬 상수를 사용)
			double newRed = (originalRed * SEP_R_R) + (originalGreen * SEP_R_G) + (originalBlue * SEP_R_B);
			double newGreen = (originalRed * SEP_G_R) + (originalGreen * SEP_G_G) + (originalBlue * SEP_B_B);
			double newBlue = (originalRed * SEP_B_R) + (originalGreen * SEP_B_G) + (originalBlue * SEP_B_B);

			// 7. 클램핑(Clamping): 세피아 공식은 255를 초과하는 경우가 많습니다.
			color.rgbRed = (BYTE)std::min(255.0, std::max(0.0, newRed));
			color.rgbGreen = (BYTE)std::min(255.0, std::max(0.0, newGreen));
			color.rgbBlue = (BYTE)std::min(255.0, std::max(0.0, newBlue));

			// 8. 계산된 새 색상 값으로 (x, y) 위치의 픽셀을 덮어씁니다.
			m_pImage->SetPixelColor(x, y, color);
		}
	}

	// 9. 모든 뷰(View)에 화면을 새로 고침하라고 알립니다.
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodWarm()
{
	// --- 필터 튜닝 값 (상수) ---
	const int nRedAdjust = 25;
	const int nGreenAdjust = 10;
	const int nBlueAdjust = -15;

	// 1. 이미지가 열려있는지 확인합니다.
	if (!m_pImage) return;

	// 2. 컬러 이미지가 아니면(그레이스케일이면) 필터를 적용할 수 없습니다.
	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Warm filter can only be applied to color images."));
		return;
	}

	// 3. 이미지의 가로, 세로 크기를 가져옵니다.
	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 4. 모든 픽셀을 순회하기 위한 2중 반복문
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 5. (x, y) 위치의 픽셀 색상 값(RGB)을 읽어옵니다.
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			// 6. Warm Tone 로직 적용 (R, G는 높이고, B는 낮춥니다)
			//    (값은 원하는 느낌에 따라 조절(tune)할 수 있습니다.)
			int newRed = color.rgbRed + nRedAdjust;
			int newGreen = color.rgbGreen + nGreenAdjust;
			int newBlue = color.rgbBlue + nBlueAdjust;

			// 7. 클램핑(Clamping): 색상 값은 0~255 범위를 벗어날 수 없습니다.
			color.rgbRed = (BYTE)std::min(255, std::max(0, newRed));
			color.rgbGreen = (BYTE)std::min(255, std::max(0, newGreen));
			color.rgbBlue = (BYTE)std::min(255, std::max(0, newBlue));

			// 8. 계산된 새 색상 값으로 (x, y) 위치의 픽셀을 덮어씁니다.
			m_pImage->SetPixelColor(x, y, color);
		}
	}

	// 9. 모든 뷰(View)에 화면을 새로 고침하라고 알립니다.
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodCool()
{
	// --- 필터 튜닝 값 (상수) ---
	const int nRedAdjust = -15;
	const int nGreenAdjust = -10;
	const int nBlueAdjust = 25;

	// 1. 이미지가 열려있는지 확인합니다.
	if (!m_pImage) return;

	// 2. 컬러 이미지가 아니면(그레이스케일이면) 필터를 적용할 수 없습니다.
	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Cool filter can only be applied to color images."));
		return;
	}

	// 3. 이미지의 가로, 세로 크기를 가져옵니다.
	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 4. 모든 픽셀을 순회하기 위한 2중 반복문
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 5. (x, y) 위치의 픽셀 색상 값(RGB)을 읽어옵니다.
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			// 6. Cool Tone 로직 적용 (B는 높이고, R, G는 낮춥니다)
			int newRed = color.rgbRed + nRedAdjust;
			int newGreen = color.rgbGreen + nGreenAdjust;
			int newBlue = color.rgbBlue + nBlueAdjust;

			// 7. 클램핑(Clamping): 색상 값은 0~255 범위를 벗어날 수 없습니다.
			color.rgbRed = (BYTE)std::min(255, std::max(0, newRed));
			color.rgbGreen = (BYTE)std::min(255, std::max(0, newGreen));
			color.rgbBlue = (BYTE)std::min(255, std::max(0, newBlue));

			// 8. 계산된 새 색상 값으로 (x, y) 위치의 픽셀을 덮어씁니다.
			m_pImage->SetPixelColor(x, y, color);
		}
	}

	// 9. 모든 뷰(View)에 화면을 새로 고침하라고 알립니다.
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodBloom()
{
	// TODO: 블룸 필터 구현 코드 추가
	if (!m_pImage) return;
	AfxMessageBox(_T("Bloom Filter 실행"));
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodVignette()
{
	// --- 필터 튜닝 값 (상수) ---
	// 비네트의 "강도"입니다.
	// 1.0 : 이미지 모서리(가장자리)에서 정확히 어두워지기 시작합니다.
	// 1.5 : 모서리보다 더 안쪽(중심 쪽)에서부터 어두워집니다. (효과가 강해짐)
	// 0.7 : 모서리 바깥쪽부터 어두워져서, 실제 이미지엔 효과가 약하게 들어갑니다.
	const double VIGNETTE_STRENGTH = 1.3;
	// --------------------------

	// 1. 이미지가 열려있는지 확인합니다.
	if (!m_pImage) return;

	// 2. 컬러/그레이스케일 모두 적용 가능하므로 Bpp 체크는 생략합니다.

	// 3. 이미지 크기 및 중심점 계산
	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 중심점 (소수점 계산을 위해 double 사용)
	double centerX = width / 2.0;
	double centerY = height / 2.0;

	// 4. 중심(center)에서 가장 먼 지점(모서리)까지의 최대 거리 계산
	// (피타고라스 정리: (0,0) ~ (centerX, centerY) 거리)
	double maxDistance = sqrt((centerX * centerX) + (centerY * centerY));

	// 5. 모든 픽셀을 순회하기 위한 2중 반복문
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 6. 현재 픽셀(x,y)과 중심(centerX, centerY) 사이의 거리 계산
			double currentDistance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));

			// 7. Vignette 로직 (핵심)
			// (현재거리 / 최대거리)로 0.0(중심) ~ 1.0(모서리)의 비율을 구함
			// 여기에 강도를 곱해서 효과를 조절
			double ratio = (currentDistance / maxDistance) * VIGNETTE_STRENGTH;

			// 1.0에서 이 비율을 빼서 '밝기 계수(Factor)'를 구함
			// (중심: 1.0, 모서리: 0.0 근처)
			double brightnessFactor = 1.0 - ratio;

			// 8. 밝기 계수 클램핑 (0.0 ~ 1.0)
			// VIGNETTE_STRENGTH가 1.0보다 크면 계수가 음수가 될 수 있으므로,
			// 0.0 ~ 1.0 사이의 값으로 고정합니다.
			brightnessFactor = std::min(1.0, std::max(0.0, brightnessFactor));

			// 9. 픽셀 값 읽기 및 적용
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			// R, G, B 각 채널에 동일한 밝기 계수를 곱해서 어둡게 만듭니다.
			color.rgbRed = (BYTE)(color.rgbRed * brightnessFactor);
			color.rgbGreen = (BYTE)(color.rgbGreen * brightnessFactor);
			color.rgbBlue = (BYTE)(color.rgbBlue * brightnessFactor);

			// 10. 계산된 새 색상 값으로 (x, y) 위치의 픽셀을 덮어씁니다.
			m_pImage->SetPixelColor(x, y, color);
		}
	}

	// 11. 모든 뷰(View)에 화면을 새로 고침하라고 알립니다.
	UpdateAllViews(NULL);
}