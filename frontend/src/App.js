import React from 'react';
import { BrowserRouter, Routes, Route, Link, Navigate } from 'react-router-dom';
import './App.css';
import RegisterPage from './components/RegisterPage';
import LoginPage from './components/LoginPage';

const Home = () => (
  <div className="container">
    <div className="card">
      <div className="header">
        <h1>AtriumMarket</h1>
      </div>
      <div className="card-body" style={{ padding: '2rem', textAlign: 'center' }}>
        <Link to="/login" className="btn btn-full" style={{ marginBottom: '1rem' }}>
          Войти
        </Link>
        <Link to="/register" className="btn btn-full">
          Зарегистрироваться
        </Link>
      </div>
    </div>
  </div>
);

const App = () => (
  <BrowserRouter>
    <Routes>
      <Route path="/" element={<Home />} />
      <Route path="/login" element={<LoginPage />} />
      <Route path="/register" element={<RegisterPage />} />
      <Route path="*" element={<Navigate to="/" replace />} />
    </Routes>
  </BrowserRouter>
);

export default App;
