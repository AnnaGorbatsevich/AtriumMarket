import React, { useState } from 'react';
import { useAuth, getToken } from '../auth';
import { authPost } from '../api';
import Field from './Field';

const updateProfile = (payload) => authPost('/update_profile', getToken(), payload);

const toForm = (user) => ({
  fullName: user.fullName || '',
  phone: user.phone || '',
  companyName: user.companyName || '',
  taxId: user.taxId || '',
  address: user.address || '',
  description: user.description || '',
});

const ProfilePage = () => {
  const { user, updateUser } = useAuth();
  const isSeller = user.role === 'seller';

  const [editing, setEditing] = useState(false);
  const [form, setForm] = useState(() => toForm(user));
  const [errors, setErrors] = useState({});
  const [submitting, setSubmitting] = useState(false);
  const [submitError, setSubmitError] = useState('');

  const handleChange = (field) => (e) => {
    setForm((prev) => ({ ...prev, [field]: e.target.value }));
  };

  const startEditing = () => {
    setForm(toForm(user));
    setErrors({});
    setSubmitError('');
    setEditing(true);
  };

  const validate = () => {
    const next = {};
    if (!form.fullName.trim()) next.fullName = 'Укажите ФИО';
    if (!form.phone.trim()) next.phone = 'Укажите телефон';
    if (isSeller && !form.companyName.trim()) next.companyName = 'Укажите название магазина';
    if (isSeller && !form.taxId.trim()) next.taxId = 'Укажите ИНН';
    if (!isSeller && !form.address.trim()) next.address = 'Укажите адрес доставки';

    setErrors(next);
    return Object.keys(next).length === 0;
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!validate()) return;

    setSubmitError('');
    setSubmitting(true);
    try {
      const profile = await updateProfile({
        fullName: form.fullName.trim(),
        phone: form.phone.trim(),
        ...(isSeller
          ? {
              companyName: form.companyName.trim(),
              taxId: form.taxId.trim(),
              description: form.description.trim(),
            }
          : { address: form.address.trim() }),
      });
      updateUser(profile);
      setEditing(false);
    } catch (err) {
      setSubmitError(err.message);
    } finally {
      setSubmitting(false);
    }
  };

  const rows = [
    ['Роль', isSeller ? 'Продавец' : 'Покупатель'],
    ['Email', user.email],
    ...(isSeller ? [['Название магазина', user.companyName]] : []),
    [isSeller ? 'Контактное лицо' : 'ФИО', user.fullName],
    ['Телефон', user.phone],
    ...(isSeller
      ? [
          ['ИНН', user.taxId],
          ['Описание магазина', user.description],
        ]
      : [['Адрес доставки', user.address]]),
  ];

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Профиль</h1>
          {!editing && (
            <button type="button" className="change-role-btn" onClick={startEditing}>
              Редактировать
            </button>
          )}
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          {!editing &&
            rows.map(([label, value]) => (
              <p key={label} style={{ margin: '0 0 0.75rem' }}>
                <strong>{label}:</strong> {value || '—'}
              </p>
            ))}

          {editing && (
            <form onSubmit={handleSubmit} noValidate>
              {isSeller && (
                <Field label="Название магазина" error={errors.companyName}>
                  <input
                    type="text"
                    className="form-control"
                    value={form.companyName}
                    onChange={handleChange('companyName')}
                  />
                </Field>
              )}

              <Field label={isSeller ? 'Контактное лицо' : 'ФИО'} error={errors.fullName}>
                <input
                  type="text"
                  className="form-control"
                  value={form.fullName}
                  onChange={handleChange('fullName')}
                />
              </Field>

              <Field label="Email">
                <input type="email" className="form-control" value={user.email} disabled />
              </Field>

              <Field label="Телефон" error={errors.phone}>
                <input type="tel" className="form-control" value={form.phone} onChange={handleChange('phone')} />
              </Field>

              {isSeller ? (
                <>
                  <Field label="ИНН" error={errors.taxId}>
                    <input
                      type="text"
                      className="form-control"
                      value={form.taxId}
                      onChange={handleChange('taxId')}
                    />
                  </Field>
                  <Field label="Описание магазина">
                    <textarea
                      className="form-control"
                      rows="3"
                      value={form.description}
                      onChange={handleChange('description')}
                    />
                  </Field>
                </>
              ) : (
                <Field label="Адрес доставки" error={errors.address}>
                  <input
                    type="text"
                    className="form-control"
                    value={form.address}
                    onChange={handleChange('address')}
                  />
                </Field>
              )}

              {submitError && <div className="field-error submit-error">{submitError}</div>}

              <div style={{ display: 'flex', gap: '0.75rem' }}>
                <button
                  type="button"
                  className="btn"
                  style={{ background: '#e2e8f0', color: '#1e293b' }}
                  disabled={submitting}
                  onClick={() => setEditing(false)}
                >
                  Отмена
                </button>
                <button type="submit" className="btn btn-full" disabled={submitting}>
                  {submitting ? 'Сохранение...' : 'Сохранить'}
                </button>
              </div>
            </form>
          )}
        </div>
      </div>
    </div>
  );
};

export default ProfilePage;
