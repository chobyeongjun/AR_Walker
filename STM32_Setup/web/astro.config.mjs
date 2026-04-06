import { defineConfig } from 'astro/config';
import tailwind from '@astrojs/tailwind';

export default defineConfig({
  site: 'https://techblips.com',
  integrations: [tailwind()],
  markdown: {
    shikiConfig: {
      theme: 'dracula',
    },
  },
});
