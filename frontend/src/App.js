import React from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import './App.css';
import RegisterPage from './components/RegisterPage';
import LoginPage from './components/LoginPage';
import { AuthProvider, useAuth } from './auth';

const Home = () => {
  const { user, logout } = useAuth();
  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>AtriumMarket</h1>
        </div>
        <div className="card-body" style={{ padding: '2rem', textAlign: 'center' }}>
          <p className="success-subtext">
            Вы в аккаунте: {user.fullName} ({user.email})
          </p>
          <button type="button" className="btn btn-full" onClick={logout}>
            Выйти
          </button>
        </div>
      </div>
    </div>
  );
};

const RequireAuth = ({ children }) => {
  const { status, user } = useAuth();
  if (status === 'loading') return null;
  return user ? children : <Navigate to="/login" replace />;
};

const RequireGuest = ({ children }) => {
  const { status, user } = useAuth();
  if (status === 'loading') return null;
  return user ? <Navigate to="/" replace /> : children;
};

const App = () => (
  <AuthProvider>
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<RequireAuth><Home /></RequireAuth>} />
        <Route path="/login" element={<RequireGuest><LoginPage /></RequireGuest>} />
        <Route path="/register" element={<RequireGuest><RegisterPage /></RequireGuest>} />
        <Route path="*" element={<Navigate to="/" replace />} />
      </Routes>
    </BrowserRouter>
  </AuthProvider>
);

export default App;
