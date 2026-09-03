import React from 'react';
import { useNavigate } from 'react-router-dom';
import LoginForm from './LoginForm';
import { useAuth } from '../auth';

const LoginPage = () => {
  const { login } = useAuth();
  const navigate = useNavigate();

  const handleLoggedIn = async (data) => {
    await login(data.token);
    navigate('/');
  };

  return <LoginForm onSubmitted={handleLoggedIn} />;
};

export default LoginPage;
