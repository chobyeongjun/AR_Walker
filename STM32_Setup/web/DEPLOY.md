# TechBlips STM32 학습 가이드 — 배포 안내

## 기술 스택

- **Next.js** (App Router) — 정적 사이트 빌더 (static export)
- **Tailwind CSS** + @tailwindcss/typography — 스타일링
- **next-mdx-remote** — 마크다운/MDX 렌더링
- **Cloudflare Pages** — 배포

## 로컬 개발

```bash
cd STM32_Setup/web

# 의존성 설치
npm install

# 개발 서버 실행
npm run dev
# → http://localhost:3000

# 프로덕션 빌드
npm run build

# 빌드 결과 미리보기
npm run start
```

## Cloudflare Pages 배포

### 방법 1: GitHub 연동 (권장)

1. GitHub에 이 프로젝트를 push
2. Cloudflare Dashboard → Pages → Create a project → Connect to Git
3. 설정:
   - Build command: `npm run build`
   - Build output directory: `out`
   - Root directory: `STM32_Setup/web` (모노레포인 경우)
   - Node.js version: 18
4. Save and Deploy

### 방법 2: Wrangler CLI

```bash
npm run build
npx wrangler pages deploy out --project-name=techblips
```

## 이미지 추가

Gemini로 생성한 이미지를 `public/images/study/stm32/` 폴더에 배치합니다.
마크다운에서 `![alt](/images/study/stm32/filename.png)` 형식으로 참조합니다.

필요한 이미지 목록:
1. `stm32-hero.png` — 히어로 커버 (1200x630)
2. `cortex-m7-block.png` — Cortex-M7 블록 다이어그램
3. `memory-map.png` — 메모리 맵
4. `bus-domains.png` — 버스 도메인
5. `lqfp100-pinout.png` — LQFP-100 핀아웃
6. `af-mux.png` — AF 멀티플렉서
7. `clock-tree.png` — 클럭 트리
8. `gpio-modes.png` — GPIO 모드
9. `push-pull-od.png` — Push-Pull vs Open-Drain
10. `can-topology.png` — CAN 버스 토폴로지
11. `cubemx-pinout.png` — CubeMX 핀 설정
12. `cubemx-clock.png` — CubeMX 클럭 설정
13. `pin-mapping.png` — Teensy→STM32 매핑
14. `bringup-flow.png` — 보드 브링업 플로차트

## 커스텀 도메인 (techblips.com)

Cloudflare Pages에서 Custom domains → Add custom domain → `techblips.com` 입력.
DNS가 이미 Cloudflare에 있으면 자동 설정됩니다.
