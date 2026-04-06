import Link from "next/link";
import { getAllStudies } from "@/lib/content";
import ThemeToggle from "@/components/ThemeToggle";

export default function HomePage() {
  const studies = getAllStudies()
    .filter((s) => !s.frontmatter.draft)
    .sort(
      (a, b) =>
        new Date(b.frontmatter.date).getTime() -
        new Date(a.frontmatter.date).getTime()
    );

  return (
    <>
      {/* Navigation */}
      <header className="sticky top-0 z-40 backdrop-blur-lg bg-white/80 dark:bg-page-dark/80 border-b border-gray-200 dark:border-gray-800">
        <div className="max-w-4xl mx-auto px-4 sm:px-6 h-16 flex items-center justify-between">
          <Link
            href="/"
            className="text-lg font-bold text-gray-900 dark:text-white"
          >
            <span className="text-accent dark:text-accent-dark">Tech</span>
            Blips
          </Link>
          <ThemeToggle />
        </div>
      </header>

      {/* Hero */}
      <div className="bg-gradient-to-b from-accent/5 to-transparent dark:from-accent-dark/5">
        <div className="max-w-4xl mx-auto px-4 sm:px-6 py-16 sm:py-24 text-center">
          <h1 className="text-4xl sm:text-5xl font-black tracking-tight text-gray-900 dark:text-white mb-4">
            엔지니어 스터디
          </h1>
          <p className="text-lg text-gray-600 dark:text-gray-400 max-w-xl mx-auto">
            처음부터 공부하며 기록하는 엔지니어링 학습 로그. 직접 공부하면서
            기록하는 과정 그 자체가 콘텐츠입니다.
          </p>
        </div>
      </div>

      {/* Study list */}
      <div className="max-w-4xl mx-auto px-4 sm:px-6 py-8">
        <div className="grid gap-6 sm:grid-cols-2">
          {studies.map((study) => (
            <Link
              key={study.slug}
              href={`/study/${study.slug}`}
              className="group block p-6 bg-white dark:bg-gray-800/50 border border-gray-200 dark:border-gray-700 rounded-xl hover:border-accent dark:hover:border-accent-dark hover:-translate-y-1.5 hover:shadow-lg hover:shadow-accent/10 dark:hover:shadow-accent-dark/10 transition-all duration-300"
            >
              <h2 className="text-xl font-bold text-gray-900 dark:text-white group-hover:text-accent dark:group-hover:text-accent-dark transition-colors mb-2">
                {study.frontmatter.title}
              </h2>
              <p className="text-sm text-gray-600 dark:text-gray-400 line-clamp-2 mb-4">
                {study.frontmatter.description}
              </p>
              <div className="flex flex-wrap items-center gap-3">
                <span className="text-xs text-gray-400">
                  {study.frontmatter.date}
                </span>
                {study.frontmatter.tags?.slice(0, 3).map((tag) => (
                  <span
                    key={tag}
                    className="px-2 py-0.5 text-xs font-semibold uppercase tracking-wider rounded-md bg-accent/10 text-accent dark:bg-accent-dark/10 dark:text-accent-dark"
                  >
                    {tag}
                  </span>
                ))}
              </div>
            </Link>
          ))}
        </div>

        {studies.length === 0 && (
          <p className="text-center text-gray-500 dark:text-gray-400 py-16">
            아직 학습 콘텐츠가 없습니다. 곧 추가됩니다!
          </p>
        )}
      </div>

      {/* Footer */}
      <footer className="border-t border-gray-200 dark:border-gray-800 mt-16">
        <div className="max-w-4xl mx-auto px-4 sm:px-6 py-8 text-center text-sm text-gray-500 dark:text-gray-400">
          <p>&copy; 2026 TechBlips. Built with Next.js + Tailwind CSS.</p>
        </div>
      </footer>
    </>
  );
}
