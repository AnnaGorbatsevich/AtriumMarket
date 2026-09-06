export const pluralizeVariants = (count) => {
  const mod10 = count % 10;
  const mod100 = count % 100;
  if (mod10 === 1 && mod100 !== 11) return 'вариация';
  if ([2, 3, 4].includes(mod10) && ![12, 13, 14].includes(mod100)) return 'вариации';
  return 'вариаций';
};

export const formatOptions = (options) => {
  if (!options || Object.keys(options).length === 0) return '';
  return `${Object.entries(options)
    .map(([key, value]) => `${key}: ${value}`)
    .join(', ')} — `;
};

export const minPrice = (variants) => Math.min(...variants.map((variant) => variant.price));
