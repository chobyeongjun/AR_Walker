import { notFound } from "next/navigation";
import { MDXRemote } from "next-mdx-remote/rsc";
import Link from "next/link";
import remarkGfm from "remark-gfm";
import rehypeSlug from "rehype-slug";
import {
  getAllStudies,
  getStudyBySlug,
  extractHeadings,
  estimateReadingTime,
} from "@/lib/content";
import ThemeToggle from "@/components/ThemeToggle";
import ProgressBar from "@/components/ProgressBar";
import TableOfContents from "@/components/TableOfContents";

export function generateStaticParams() {
  const studies = getAllStudies();
  return studies.map((study) => ({
    slug: study.slug,
  }));
}

export function generateMetadata({ params }: { params: { slug: string } }) {
  const study = getStudyBySlug(params.slug);
  if (!study) return { title: "Not Found" };

  return {
    title: `${study.frontmatter.title} — TechBlips`,
    description: study.frontmatter.description,
  };
}

export default function StudyPage({ params }: { params: { slug: string } }) {
  const study = getStudyBySlug(params.slug);
  if (!study) notFound();

  const headings = extractHeadings(study.content);
  const readingTime = estimateReadingTime(study.content);
  const { title, description, date, tags } = study.frontmatter;

  return (
    <>
      <ProgressBar />

      {/* Navigation */}
      <header className="sticky top-0 z-40 backdrop-blur-lg bg-white/80 dark:bg-page-dark/80 border-b border-gray-200 dark:border-gray-800">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 h-16 flex items-center justify-between">
          <Link
            href="/"
            className="flex items-center gap-2 text-lg font-bold text-gray-900 dark:text-white"
          >
            <span className="text-accent dark:text-accent-dark">Tech</span>
            Blips
          </Link>
          <div className="flex items-center gap-4">
            <Link
              href="/"
              className="text-sm text-gray-600 dark:text-gray-400 hover:text-accent dark:hover:text-accent-dark transition-colors"
            >
              Study
            </Link>
            <ThemeToggle />
          </div>
        </div>
      </header>

      {/* Hero */}
      <div className="bg-gradient-to-b from-accent/5 to-transparent dark:from-accent-dark/5 border-b border-gray-100 dark:border-gray-800">
        <div className="max-w-4xl mx-auto px-4 sm:px-6 py-12 sm:py-16">
          {/* Breadcrumb */}
          <nav className="flex items-center gap-2 text-sm text-gray-500 dark:text-gray-400 mb-6">
            <Link
              href="/"
              className="hover:text-accent dark:hover:text-accent-dark transition-colors"
            >
              홈
            </Link>
            <span>/</span>
            <Link
              href="/"
              className="hover:text-accent dark:hover:text-accent-dark transition-colors"
            >
              엔지니어 스터디
            </Link>
            <span>/</span>
            <span className="text-gray-700 dark:text-gray-300">STM32</span>
          </nav>

          <h1 className="text-3xl sm:text-4xl lg:text-5xl font-black tracking-tight text-gray-900 dark:text-white leading-tight mb-4">
            {title}
          </h1>
          <p className="text-lg text-gray-600 dark:text-gray-400 max-w-2xl mb-6">
            {description}
          </p>

          <div className="flex flex-wrap items-center gap-4 text-sm text-gray-500 dark:text-gray-400">
            {date && (
              <span className="flex items-center gap-1.5">
                <svg
                  className="w-4 h-4"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    strokeWidth={2}
                    d="M8 7V3m8 4V3m-9 8h10M5 21h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v12a2 2 0 002 2z"
                  />
                </svg>
                {date}
              </span>
            )}
            <span className="flex items-center gap-1.5">
              <svg
                className="w-4 h-4"
                fill="none"
                stroke="currentColor"
                viewBox="0 0 24 24"
              >
                <path
                  strokeLinecap="round"
                  strokeLinejoin="round"
                  strokeWidth={2}
                  d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z"
                />
              </svg>
              약 {readingTime}분
            </span>
          </div>

          {tags && tags.length > 0 && (
            <div className="flex flex-wrap gap-2 mt-4">
              {tags.map((tag) => (
                <span
                  key={tag}
                  className="px-3 py-1 text-xs font-semibold uppercase tracking-wider rounded-full bg-accent/10 text-accent dark:bg-accent-dark/10 dark:text-accent-dark"
                >
                  {tag}
                </span>
              ))}
            </div>
          )}
        </div>
      </div>

      {/* Content area with TOC sidebar */}
      <div className="max-w-7xl mx-auto px-4 sm:px-6 py-8 lg:py-12">
        <div className="xl:grid xl:grid-cols-[280px_1fr] xl:gap-12">
          {/* Sidebar TOC (sticky on desktop) */}
          <aside className="xl:block mb-8 xl:mb-0">
            <div className="xl:sticky xl:top-24 xl:max-h-[calc(100vh-120px)] xl:overflow-y-auto xl:pr-4">
              <h2 className="hidden xl:block text-xs font-bold uppercase tracking-widest text-gray-400 dark:text-gray-500 mb-4 px-3">
                목차
              </h2>
              <TableOfContents headings={headings} />
            </div>
          </aside>

          {/* Main content */}
          <article className="prose prose-lg dark:prose-invert max-w-none">
            <MDXRemote
              source={study.content}
              options={{
                mdxOptions: {
                  remarkPlugins: [remarkGfm],
                  rehypePlugins: [rehypeSlug],
                },
              }}
            />
          </article>
        </div>
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
