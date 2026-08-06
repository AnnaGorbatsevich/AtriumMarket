import React, { useState } from 'react';
import Field from './Field';

const GATEWAY_BASE_URL = process.env.GATEWAY_BASE_URL;

const registerUser = async (payload) => {
  const response = await fetch(`${GATEWAY_BASE_URL}/register`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload),
  });
  if (!response.ok) throw new Error(`Error: ${response.status}`);
  return response.json();
};

const RegistrationForm = ({ role, onChangeRole, onSubmitted }) => {
  const isSeller = role === 'seller';

  const [form, setForm] = useState({
    fullName: '',
    companyName: '',
    email: '',
    phone: '',
    taxId: '',
    address: '',
    description: '',
    password: '',
    confirmPassword: '',
  });
  const [errors, setErrors] = useState({});
  const [submitting, setSubmitting] = useState(false);
  const [submitError, setSubmitError] = useState('');

  const handleChange = (field) => (e) => {
    setForm((prev) => ({ ...prev, [field]: e.target.value }));
  };

  const validate = () => {
    const next = {};
    if (!form.fullName.trim()) next.fullName = 'Укажите ФИО';
    if (isSeller && !form.companyName.trim()) next.companyName = 'Укажите название магазина';
    if (!form.email.trim()) next.email = 'Укажите email';
    else if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(form.email)) next.email = 'Некорректный email';
    if (!form.phone.trim()) next.phone = 'Укажите телефон';
    if (isSeller && !form.taxId.trim()) next.taxId = 'Укажите ИНН';
    if (!isSeller && !form.address.trim()) next.address = 'Укажите адрес доставки';
    if (!form.password) next.password = 'Укажите пароль';
    else if (form.password.length < 6) next.password = 'Минимум 6 символов';
    if (form.confirmPassword !== form.password) next.confirmPassword = 'Пароли не совпадают';

    setErrors(next);
    return Object.keys(next).length === 0;
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!validate()) return;

    const name = isSeller ? form.companyName : form.fullName;
    setSubmitError('');
    setSubmitting(true);
    try {
      await registerUser({
        role,
        fullName: form.fullName,
        companyName: isSeller ? form.companyName : undefined,
        email: form.email,
        phone: form.phone,
        taxId: isSeller ? form.taxId : undefined,
        address: !isSeller ? form.address : undefined,
        description: isSeller ? form.description : undefined,
        password: form.password,
      });
      onSubmitted({ role, email: form.email, name });
    } catch (err) {
      setSubmitError('Не удалось отправить данные на сервер. Попробуйте позже.');
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Регистрация {isSeller ? 'продавца' : 'покупателя'}</h1>
          <button type="button" className="change-role-btn" onClick={onChangeRole}>
            Изменить тип
          </button>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          <form onSubmit={handleSubmit} noValidate>
            {isSeller && (
              <Field label="Название магазина" error={errors.companyName}>
                <input
                  type="text"
                  className="form-control"
                  value={form.companyName}
                  onChange={handleChange('companyName')}
                  placeholder="ООО «Ромашка»"
                />
              </Field>
            )}

            <Field label={isSeller ? 'Контактное лицо' : 'ФИО'} error={errors.fullName}>
              <input
                type="text"
                className="form-control"
                value={form.fullName}
                onChange={handleChange('fullName')}
                placeholder="Иванов Иван Иванович"
              />
            </Field>

            <Field label="Email" error={errors.email}>
              <input
                type="email"
                className="form-control"
                value={form.email}
                onChange={handleChange('email')}
                placeholder="name@example.com"
              />
            </Field>

            <Field label="Телефон" error={errors.phone}>
              <input
                type="tel"
                className="form-control"
                value={form.phone}
                onChange={handleChange('phone')}
                placeholder="+7 (900) 000-00-00"
              />
            </Field>

            {isSeller ? (
              <>
                <Field label="ИНН" error={errors.taxId}>
                  <input
                    type="text"
                    className="form-control"
                    value={form.taxId}
                    onChange={handleChange('taxId')}
                    placeholder="000000000000"
                  />
                </Field>
                <Field label="Описание магазина">
                  <textarea
                    className="form-control"
                    rows="3"
                    value={form.description}
                    onChange={handleChange('description')}
                    placeholder="Коротко расскажите о вашем магазине"
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
                  placeholder="Город, улица, дом, квартира"
                />
              </Field>
            )}

            <Field label="Пароль" error={errors.password}>
              <input
                type="password"
                className="form-control"
                value={form.password}
                onChange={handleChange('password')}
                placeholder="Минимум 6 символов"
              />
            </Field>

            <Field label="Подтверждение пароля" error={errors.confirmPassword}>
              <input
                type="password"
                className="form-control"
                value={form.confirmPassword}
                onChange={handleChange('confirmPassword')}
                placeholder="Повторите пароль"
              />
            </Field>

            {submitError && <div className="field-error submit-error">{submitError}</div>}

            <button type="submit" className="btn btn-full" disabled={submitting}>
              {submitting ? 'Отправка...' : 'Зарегистрироваться'}
            </button>
          </form>
        </div>
      </div>
    </div>
  );
};

export default RegistrationForm;
