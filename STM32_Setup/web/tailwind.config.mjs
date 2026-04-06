/** @type {import('tailwindcss').Config} */
export default {
  content: [
    './app/**/*.{js,ts,jsx,tsx,mdx}',
    './components/**/*.{js,ts,jsx,tsx}',
    './content/**/*.{md,mdx}',
  ],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        accent: {
          DEFAULT: '#6c5ce7',
          hover: '#5a4bd1',
          light: '#ede9fe',
          dark: '#818cf8',
        },
        surface: {
          light: '#ffffff',
          dark: '#0f172a',
        },
        page: {
          light: '#fafbfc',
          dark: '#0b1120',
        },
      },
      fontFamily: {
        sans: ['Inter', '-apple-system', 'BlinkMacSystemFont', 'Segoe UI', 'sans-serif'],
        mono: ['ui-monospace', 'SFMono-Regular', 'Menlo', 'Monaco', 'Consolas', 'monospace'],
      },
      typography: ({ theme }) => ({
        DEFAULT: {
          css: {
            '--tw-prose-body': '#1a1a2e',
            '--tw-prose-headings': '#1a1a2e',
            '--tw-prose-links': '#6c5ce7',
            maxWidth: '720px',
            lineHeight: '1.8',
            fontSize: '1.1rem',
            h2: {
              fontSize: '1.8rem',
              fontWeight: '800',
              marginTop: '3rem',
              borderBottom: `1px solid ${theme('colors.gray.200')}`,
              paddingBottom: '0.5rem',
            },
            h3: {
              fontSize: '1.4rem',
              fontWeight: '700',
              color: '#6c5ce7',
              marginTop: '2rem',
            },
            h4: {
              fontSize: '1.15rem',
              fontWeight: '600',
              marginTop: '1.5rem',
            },
            a: {
              color: '#6c5ce7',
              textDecoration: 'underline',
              textUnderlineOffset: '4px',
            },
            blockquote: {
              borderLeftColor: '#6c5ce7',
              backgroundColor: '#ede9fe',
              borderRadius: '0 12px 12px 0',
              padding: '1rem 1.5rem',
              fontStyle: 'normal',
            },
            code: {
              backgroundColor: 'rgba(108, 92, 231, 0.06)',
              color: '#6c5ce7',
              padding: '0.2em 0.4em',
              borderRadius: '6px',
              fontWeight: '400',
            },
            'code::before': { content: 'none' },
            'code::after': { content: 'none' },
            pre: {
              backgroundColor: '#1e1e2e',
              borderRadius: '12px',
              padding: '1.5rem',
            },
            'pre code': {
              backgroundColor: 'transparent',
              color: 'inherit',
              padding: '0',
            },
            table: {
              fontSize: '0.95rem',
              borderRadius: '12px',
              overflow: 'hidden',
            },
            thead: {
              backgroundColor: 'rgba(108, 92, 231, 0.06)',
            },
            th: {
              fontWeight: '700',
              fontSize: '0.85rem',
              textTransform: 'uppercase',
              letterSpacing: '0.04em',
              color: '#6c5ce7',
            },
            'tbody tr': {
              borderBottomColor: theme('colors.gray.200'),
            },
          },
        },
        invert: {
          css: {
            '--tw-prose-body': '#e2e8f0',
            '--tw-prose-headings': '#e2e8f0',
            '--tw-prose-links': '#818cf8',
            h3: { color: '#818cf8' },
            a: { color: '#818cf8' },
            blockquote: {
              borderLeftColor: '#818cf8',
              backgroundColor: 'rgba(129, 140, 248, 0.08)',
            },
            code: {
              backgroundColor: 'rgba(129, 140, 248, 0.06)',
              color: '#818cf8',
            },
            th: { color: '#818cf8' },
            thead: {
              backgroundColor: 'rgba(129, 140, 248, 0.08)',
            },
            'tbody tr': {
              borderBottomColor: '#334155',
            },
            'tbody tr:hover': {
              backgroundColor: 'rgba(129, 140, 248, 0.06)',
            },
          },
        },
      }),
    },
  },
  plugins: [
    require('@tailwindcss/typography'),
  ],
};
