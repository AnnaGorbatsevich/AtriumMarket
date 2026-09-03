import React, { useState } from 'react';
import { Link } from 'react-router-dom';
import Field from './Field';
import { postJson } from '../api';

const loginUser = (payload) => postJson('/login', payload);

const LoginForm = ({onSubmitted }) => {

  const [form, setForm] = useState({
    email: '',
    password: '',
  });
  const [errors, setErrors] = useState({});
  const [submitting, setSubmitting] = useState(false);
  const [submitError, setSubmitError] = useState('');

  const handleChange = (field) => (e) => {
    setForm((prev) => ({ ...prev, [field]: e.target.value }));
  };


  const handleSubmit = async (e) => {
    e.preventDefault();

    setSubmitError('');
    setSubmitting(true);
    try {
      const data = await loginUser({
        email: form.email,
        password: form.password,
      });
      onSubmitted(data);
    } catch (err) {
      setSubmitError(err.message);
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Вход </h1>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          <form onSubmit={handleSubmit} noValidate>


            <Field label="Email" error={errors.email}>
              <input
                type="email"
                className="form-control"
                value={form.email}
                onChange={handleChange('email')}
                placeholder="name@example.com"
              />
            </Field>


            <Field label="Пароль" error={errors.password}>
              <input
                type="password"
                className="form-control"
                value={form.password}
                onChange={handleChange('password')}
              />
            </Field>


            {submitError && <div className="field-error submit-error">{submitError}</div>}

            <button type="submit" className="btn btn-full" disabled={submitting}>
              {submitting ? 'Отправка...' : 'Войти'}
            </button>

            <p className="success-subtext" style={{ textAlign: 'center', marginTop: '1rem' }}>
              Нет аккаунта? <Link to="/register">Зарегистрироваться</Link>
            </p>
          </form>
        </div>
      </div>
    </div>
  );
};

export default LoginForm;
