import fs from "fs";
import path from "path";
import matter from "gray-matter";

export interface StudyFrontmatter {
  title: string;
  description: string;
  date: string;
  tags?: string[];
  draft?: boolean;
}

export interface StudyEntry {
  slug: string;
  frontmatter: StudyFrontmatter;
  content: string;
}

export interface Heading {
  depth: number;
  slug: string;
  text: string;
}

const CONTENT_DIR = path.join(process.cwd(), "content", "study");

export function getAllStudies(): StudyEntry[] {
  if (!fs.existsSync(CONTENT_DIR)) return [];

  const files = fs.readdirSync(CONTENT_DIR).filter((f) => f.endsWith(".md") || f.endsWith(".mdx"));

  return files.map((file) => {
    const raw = fs.readFileSync(path.join(CONTENT_DIR, file), "utf-8");
    const { data, content } = matter(raw);

    return {
      slug: file.replace(/\.mdx?$/, ""),
      frontmatter: data as StudyFrontmatter,
      content,
    };
  });
}

export function getStudyBySlug(slug: string): StudyEntry | null {
  const mdPath = path.join(CONTENT_DIR, `${slug}.md`);
  const mdxPath = path.join(CONTENT_DIR, `${slug}.mdx`);

  let filePath: string | null = null;
  if (fs.existsSync(mdPath)) filePath = mdPath;
  else if (fs.existsSync(mdxPath)) filePath = mdxPath;
  else return null;

  const raw = fs.readFileSync(filePath, "utf-8");
  const { data, content } = matter(raw);

  return {
    slug,
    frontmatter: data as StudyFrontmatter,
    content,
  };
}

function slugify(text: string): string {
  return text
    .toLowerCase()
    .replace(/[^\w\s\uAC00-\uD7A3-]/g, "")
    .replace(/\s+/g, "-")
    .replace(/-+/g, "-")
    .trim();
}

export function extractHeadings(markdown: string): Heading[] {
  const headings: Heading[] = [];
  const lines = markdown.split("\n");
  let inCodeBlock = false;

  for (const line of lines) {
    if (line.trim().startsWith("```")) {
      inCodeBlock = !inCodeBlock;
      continue;
    }
    if (inCodeBlock) continue;

    const match = line.match(/^(#{2,3})\s+(.+)$/);
    if (match) {
      const depth = match[1].length;
      const text = match[2]
        .replace(/\*\*/g, "")
        .replace(/\*/g, "")
        .replace(/`/g, "")
        .trim();
      headings.push({
        depth,
        slug: slugify(text),
        text,
      });
    }
  }

  return headings;
}

export function estimateReadingTime(content: string): number {
  // Korean text: roughly 500 chars/min; mixed content estimate
  const charCount = content.replace(/\s/g, "").length;
  return Math.max(1, Math.ceil(charCount / 500));
}
