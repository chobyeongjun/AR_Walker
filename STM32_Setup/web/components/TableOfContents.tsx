"use client";

import { useState, useEffect, useCallback } from "react";

interface Heading {
  depth: number;
  slug: string;
  text: string;
}

interface TableOfContentsProps {
  headings: Heading[];
}

export default function TableOfContents({ headings }: TableOfContentsProps) {
  const [activeId, setActiveId] = useState<string>("");
  const [isOpen, setIsOpen] = useState(false);

  const tocHeadings = headings.filter((h) => h.depth === 2 || h.depth === 3);

  // Scroll spy
  useEffect(() => {
    const headingEls = document.querySelectorAll("article h2[id], article h3[id]");
    if (headingEls.length === 0) return;

    const observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            setActiveId(entry.target.id);
          }
        });
      },
      { rootMargin: "-80px 0px -60% 0px", threshold: 0 }
    );

    headingEls.forEach((h) => observer.observe(h));
    return () => observer.disconnect();
  }, []);

  const handleLinkClick = useCallback(() => {
    if (window.innerWidth < 1280) {
      setIsOpen(false);
    }
  }, []);

  return (
    <nav className="toc-nav">
      <button
        onClick={() => setIsOpen(!isOpen)}
        className="xl:hidden flex items-center gap-2 w-full px-4 py-3 bg-white dark:bg-gray-800 border border-gray-200 dark:border-gray-700 rounded-xl text-sm font-semibold text-gray-700 dark:text-gray-200 mb-4"
      >
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
            d="M4 6h16M4 12h16M4 18h16"
          />
        </svg>
        목차
        <svg
          className={`w-4 h-4 ml-auto transition-transform ${isOpen ? "rotate-180" : ""}`}
          fill="none"
          stroke="currentColor"
          viewBox="0 0 24 24"
        >
          <path
            strokeLinecap="round"
            strokeLinejoin="round"
            strokeWidth={2}
            d="M19 9l-7 7-7-7"
          />
        </svg>
      </button>
      <ul
        className={`space-y-1 text-sm ${isOpen ? "block" : "hidden"} xl:block`}
      >
        {tocHeadings.map((h) => (
          <li key={h.slug} className={h.depth === 3 ? "ml-4" : ""}>
            <a
              href={`#${h.slug}`}
              onClick={handleLinkClick}
              className={`toc-link block px-3 py-1.5 rounded-lg border-l-2 border-transparent text-gray-600 dark:text-gray-400 hover:text-accent dark:hover:text-accent-dark hover:bg-accent/5 transition-all duration-200 ${
                h.depth === 2 ? "font-medium" : "text-xs"
              } ${activeId === h.slug ? "active" : ""}`}
              data-heading={h.slug}
            >
              {h.text}
            </a>
          </li>
        ))}
      </ul>
    </nav>
  );
}
