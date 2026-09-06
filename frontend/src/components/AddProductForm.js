import React, { useEffect, useState } from 'react';
import Field from './Field';
import { authPost, getJson } from '../api';
import { getToken } from '../auth';

const addProduct = (payload) => authPost('/add_product', getToken(), payload);

const emptyVariant = () => ({ price: '', quantity: '', optionsText: '' });

const parseOptionsText = (text) => {
  const result = {};
  text.split(',').forEach((pair) => {
    const idx = pair.indexOf(':');
    if (idx === -1) return;
    const key = pair.slice(0, idx).trim();
    const value = pair.slice(idx + 1).trim();
    if (key && value) result[key] = value;
  });
  return result;
};

const isNonNegativeInteger = (value) => value !== '' && Number.isInteger(Number(value)) && Number(value) >= 0;

const AddProductForm = ({ onAdded, onCancel }) => {
  const [categories, setCategories] = useState([]);

  const [form, setForm] = useState({ name: '', description: '', categoryId: '', status: 'active' });
  const [variants, setVariants] = useState([emptyVariant()]);
  const [errors, setErrors] = useState({});
  const [submitting, setSubmitting] = useState(false);
  const [submitError, setSubmitError] = useState('');

  useEffect(() => {
    getJson('/categories')
      .then(setCategories)
      .catch(() => setCategories([]));
  }, []);

  const handleChange = (field) => (e) => {
    setForm((prev) => ({ ...prev, [field]: e.target.value }));
  };

  const handleVariantChange = (index, field) => (e) => {
    const value = e.target.value;
    setVariants((prev) => prev.map((variant, i) => (i === index ? { ...variant, [field]: value } : variant)));
  };

  const addVariantRow = () => setVariants((prev) => [...prev, emptyVariant()]);

  const removeVariantRow = (index) => setVariants((prev) => prev.filter((_, i) => i !== index));

  const validate = () => {
    const next = {};
    if (!form.name.trim()) next.name = 'Укажите название товара';

    const variantErrors = variants.map((variant) => {
      const rowErrors = {};
      if (!isNonNegativeInteger(variant.price)) rowErrors.price = 'Целое число, не меньше 0';
      if (!isNonNegativeInteger(variant.quantity)) rowErrors.quantity = 'Целое число, не меньше 0';
      return rowErrors;
    });
    if (variantErrors.some((rowErrors) => Object.keys(rowErrors).length > 0)) {
      next.variants = variantErrors;
    }

    setErrors(next);
    return Object.keys(next).length === 0;
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!validate()) return;

    setSubmitError('');
    setSubmitting(true);
    try {
      await addProduct({
        name: form.name,
        description: form.description || undefined,
        categoryId: form.categoryId ? Number(form.categoryId) : undefined,
        status: form.status,
        variants: variants.map((variant) => {
          const options = parseOptionsText(variant.optionsText);
          return {
            price: Number(variant.price),
            quantity: Number(variant.quantity),
            options: Object.keys(options).length > 0 ? options : undefined,
          };
        }),
      });
      onAdded();
    } catch (err) {
      setSubmitError(err.message);
    } finally {
      setSubmitting(false);
    }
  };

  const variantErrors = errors.variants || [];

  return (
    <form onSubmit={handleSubmit} noValidate>
      <Field label="Название" error={errors.name}>
        <input
          type="text"
          className="form-control"
          value={form.name}
          onChange={handleChange('name')}
          placeholder="Кроссовки Nike Air"
        />
      </Field>

      <Field label="Описание">
        <textarea
          className="form-control"
          rows="3"
          value={form.description}
          onChange={handleChange('description')}
          placeholder="Коротко опишите товар"
        />
      </Field>

      <Field label="Категория">
        <select className="form-control" value={form.categoryId} onChange={handleChange('categoryId')}>
          <option value="">Без категории</option>
          {categories.map((category) => (
            <option key={category.id} value={category.id}>
              {category.name}
            </option>
          ))}
        </select>
      </Field>

      <Field label="Статус">
        <select className="form-control" value={form.status} onChange={handleChange('status')}>
          <option value="active">Активен</option>
          <option value="hidden">Скрыт</option>
        </select>
      </Field>

      <label style={{ fontWeight: 500, display: 'block', marginBottom: '0.5rem' }}>Вариации товара</label>
      {variants.map((variant, index) => (
        <div className="variant-row" key={index}>
          <Field label="Цена" error={variantErrors[index]?.price}>
            <input
              type="number"
              min="0"
              step="1"
              className="form-control"
              value={variant.price}
              onChange={handleVariantChange(index, 'price')}
              placeholder="2999"
            />
          </Field>

          <Field label="Остаток" error={variantErrors[index]?.quantity}>
            <input
              type="number"
              min="0"
              step="1"
              className="form-control"
              value={variant.quantity}
              onChange={handleVariantChange(index, 'quantity')}
              placeholder="10"
            />
          </Field>

          <Field label="Опции (размер, цвет...)">
            <input
              type="text"
              className="form-control"
              value={variant.optionsText}
              onChange={handleVariantChange(index, 'optionsText')}
              placeholder="размер: M, цвет: синий"
            />
          </Field>

          <button
            type="button"
            className="remove-variant-btn"
            onClick={() => removeVariantRow(index)}
            disabled={variants.length === 1}
            title="Удалить вариацию"
          >
            ✕
          </button>
        </div>
      ))}

      <button type="button" className="add-variant-btn" onClick={addVariantRow}>
        + Добавить вариацию
      </button>

      {submitError && <div className="field-error submit-error">{submitError}</div>}

      <div style={{ display: 'flex', gap: '0.75rem' }}>
        <button type="button" className="btn" style={{ background: '#e2e8f0', color: '#1e293b' }} onClick={onCancel}>
          Отмена
        </button>
        <button type="submit" className="btn btn-full" disabled={submitting}>
          {submitting ? 'Добавление...' : 'Добавить товар'}
        </button>
      </div>
    </form>
  );
};

export default AddProductForm;
